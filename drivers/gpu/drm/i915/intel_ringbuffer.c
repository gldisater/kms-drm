/*
 * Copyright © 2008-2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Zou Nan hai <nanhai.zou@intel.com>
 *    Xiang Hai hao<haihao.xiang@intel.com>
 *
 */

#include <linux/log2.h>
#include <drm/drmP.h>
#include "i915_drv.h"
#include <drm/i915_drm.h>
#include "i915_trace.h"
#include "intel_drv.h"

/* Rough estimate of the typical request size, performing a flush,
 * set-context and then emitting the batch.
 */
#define LEGACY_REQUEST_SIZE 200

static int __intel_ring_space(int head, int tail, int size)
{
	int space = head - tail;
	if (space <= 0)
		space += size;
	return space - I915_RING_FREE_SPACE;
}

unsigned int intel_ring_update_space(struct intel_ring *ring)
{
	ring->space = __intel_ring_space(ring->head, ring->emit, ring->size);
}

static int
gen2_render_ring_flush(struct drm_i915_gem_request *req, u32 mode)
{
	u32 cmd, *cs;

	cmd = MI_FLUSH;

	if (mode & EMIT_INVALIDATE)
		cmd |= MI_READ_FLUSH;

	cs = intel_ring_begin(req, 2);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = cmd;
	*cs++ = MI_NOOP;
	intel_ring_advance(req, cs);

	return 0;
}

static int
gen4_render_ring_flush(struct drm_i915_gem_request *req, u32 mode)
{
	u32 cmd, *cs;

	/*
	 * read/write caches:
	 *
	 * I915_GEM_DOMAIN_RENDER is always invalidated, but is
	 * only flushed if MI_NO_WRITE_FLUSH is unset.  On 965, it is
	 * also flushed at 2d versus 3d pipeline switches.
	 *
	 * read-only caches:
	 *
	 * I915_GEM_DOMAIN_SAMPLER is flushed on pre-965 if
	 * MI_READ_FLUSH is set, and is always flushed on 965.
	 *
	 * I915_GEM_DOMAIN_COMMAND may not exist?
	 *
	 * I915_GEM_DOMAIN_INSTRUCTION, which exists on 965, is
	 * invalidated when MI_EXE_FLUSH is set.
	 *
	 * I915_GEM_DOMAIN_VERTEX, which exists on 965, is
	 * invalidated with every MI_FLUSH.
	 *
	 * TLBs:
	 *
	 * On 965, TLBs associated with I915_GEM_DOMAIN_COMMAND
	 * and I915_GEM_DOMAIN_CPU in are invalidated at PTE write and
	 * I915_GEM_DOMAIN_RENDER and I915_GEM_DOMAIN_SAMPLER
	 * are flushed at any MI_FLUSH.
	 */

	cmd = MI_FLUSH;
	if (mode & EMIT_INVALIDATE) {
		cmd |= MI_EXE_FLUSH;
		if (IS_G4X(req->i915) || IS_GEN5(req->i915))
			cmd |= MI_INVALIDATE_ISP;
	}

	cs = intel_ring_begin(req, 2);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = cmd;
	*cs++ = MI_NOOP;
	intel_ring_advance(req, cs);

	return 0;
}

/**
 * Emits a PIPE_CONTROL with a non-zero post-sync operation, for
 * implementing two workarounds on gen6.  From section 1.4.7.1
 * "PIPE_CONTROL" of the Sandy Bridge PRM volume 2 part 1:
 *
 * [DevSNB-C+{W/A}] Before any depth stall flush (including those
 * produced by non-pipelined state commands), software needs to first
 * send a PIPE_CONTROL with no bits set except Post-Sync Operation !=
 * 0.
 *
 * [Dev-SNB{W/A}]: Before a PIPE_CONTROL with Write Cache Flush Enable
 * =1, a PIPE_CONTROL with any non-zero post-sync-op is required.
 *
 * And the workaround for these two requires this workaround first:
 *
 * [Dev-SNB{W/A}]: Pipe-control with CS-stall bit set must be sent
 * BEFORE the pipe-control with a post-sync op and no write-cache
 * flushes.
 *
 * And this last workaround is tricky because of the requirements on
 * that bit.  From section 1.4.7.2.3 "Stall" of the Sandy Bridge PRM
 * volume 2 part 1:
 *
 *     "1 of the following must also be set:
 *      - Render Target Cache Flush Enable ([12] of DW1)
 *      - Depth Cache Flush Enable ([0] of DW1)
 *      - Stall at Pixel Scoreboard ([1] of DW1)
 *      - Depth Stall ([13] of DW1)
 *      - Post-Sync Operation ([13] of DW1)
 *      - Notify Enable ([8] of DW1)"
 *
 * The cache flushes require the workaround flush that triggered this
 * one, so we can't use it.  Depth stall would trigger the same.
 * Post-sync nonzero is what triggered this second workaround, so we
 * can't use that one either.  Notify enable is IRQs, which aren't
 * really our business.  That leaves only stall at scoreboard.
 */
static int
intel_emit_post_sync_nonzero_flush(struct drm_i915_gem_request *req)
{
	u32 scratch_addr =
		i915_ggtt_offset(req->engine->scratch) + 2 * CACHELINE_BYTES;
	u32 *cs;

	cs = intel_ring_begin(req, 6);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = GFX_OP_PIPE_CONTROL(5);
	*cs++ = PIPE_CONTROL_CS_STALL | PIPE_CONTROL_STALL_AT_SCOREBOARD;
	*cs++ = scratch_addr | PIPE_CONTROL_GLOBAL_GTT;
	*cs++ = 0; /* low dword */
	*cs++ = 0; /* high dword */
	*cs++ = MI_NOOP;
	intel_ring_advance(req, cs);

	cs = intel_ring_begin(req, 6);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = GFX_OP_PIPE_CONTROL(5);
	*cs++ = PIPE_CONTROL_QW_WRITE;
	*cs++ = scratch_addr | PIPE_CONTROL_GLOBAL_GTT;
	*cs++ = 0;
	*cs++ = 0;
	*cs++ = MI_NOOP;
	intel_ring_advance(req, cs);

	return 0;
}

static int
gen6_render_ring_flush(struct drm_i915_gem_request *req, u32 mode)
{
	u32 scratch_addr =
		i915_ggtt_offset(req->engine->scratch) + 2 * CACHELINE_BYTES;
	u32 *cs, flags = 0;
	int ret;

	/* Force SNB workarounds for PIPE_CONTROL flushes */
	ret = intel_emit_post_sync_nonzero_flush(req);
	if (ret)
		return ret;

	/* Just flush everything.  Experiments have shown that reducing the
	 * number of bits based on the write domains has little performance
	 * impact.
	 */
	if (mode & EMIT_FLUSH) {
		flags |= PIPE_CONTROL_RENDER_TARGET_CACHE_FLUSH;
		flags |= PIPE_CONTROL_DEPTH_CACHE_FLUSH;
		/*
		 * Ensure that any following seqno writes only happen
		 * when the render cache is indeed flushed.
		 */
		flags |= PIPE_CONTROL_CS_STALL;
	}
	if (mode & EMIT_INVALIDATE) {
		flags |= PIPE_CONTROL_TLB_INVALIDATE;
		flags |= PIPE_CONTROL_INSTRUCTION_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_VF_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_CONST_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_STATE_CACHE_INVALIDATE;
		/*
		 * TLB invalidate requires a post-sync write.
		 */
		flags |= PIPE_CONTROL_QW_WRITE | PIPE_CONTROL_CS_STALL;
	}

	cs = intel_ring_begin(req, 4);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = GFX_OP_PIPE_CONTROL(4);
	*cs++ = flags;
	*cs++ = scratch_addr | PIPE_CONTROL_GLOBAL_GTT;
	*cs++ = 0;
	intel_ring_advance(req, cs);

	return 0;
}

static int
gen7_render_ring_cs_stall_wa(struct drm_i915_gem_request *req)
{
	u32 *cs;

	cs = intel_ring_begin(req, 4);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = GFX_OP_PIPE_CONTROL(4);
	*cs++ = PIPE_CONTROL_CS_STALL | PIPE_CONTROL_STALL_AT_SCOREBOARD;
	*cs++ = 0;
	*cs++ = 0;
	intel_ring_advance(req, cs);

	return 0;
}

static int
gen7_render_ring_flush(struct drm_i915_gem_request *req, u32 mode)
{
	u32 scratch_addr =
		i915_ggtt_offset(req->engine->scratch) + 2 * CACHELINE_BYTES;
	u32 *cs, flags = 0;

	/*
	 * Ensure that any following seqno writes only happen when the render
	 * cache is indeed flushed.
	 *
	 * Workaround: 4th PIPE_CONTROL command (except the ones with only
	 * read-cache invalidate bits set) must have the CS_STALL bit set. We
	 * don't try to be clever and just set it unconditionally.
	 */
	flags |= PIPE_CONTROL_CS_STALL;

	/* Just flush everything.  Experiments have shown that reducing the
	 * number of bits based on the write domains has little performance
	 * impact.
	 */
	if (mode & EMIT_FLUSH) {
		flags |= PIPE_CONTROL_RENDER_TARGET_CACHE_FLUSH;
		flags |= PIPE_CONTROL_DEPTH_CACHE_FLUSH;
		flags |= PIPE_CONTROL_DC_FLUSH_ENABLE;
		flags |= PIPE_CONTROL_FLUSH_ENABLE;
	}
	if (mode & EMIT_INVALIDATE) {
		flags |= PIPE_CONTROL_TLB_INVALIDATE;
		flags |= PIPE_CONTROL_INSTRUCTION_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_VF_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_CONST_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_STATE_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_MEDIA_STATE_CLEAR;
		/*
		 * TLB invalidate requires a post-sync write.
		 */
		flags |= PIPE_CONTROL_QW_WRITE;
		flags |= PIPE_CONTROL_GLOBAL_GTT_IVB;

		flags |= PIPE_CONTROL_STALL_AT_SCOREBOARD;

		/* Workaround: we must issue a pipe_control with CS-stall bit
		 * set before a pipe_control command that has the state cache
		 * invalidate bit set. */
		gen7_render_ring_cs_stall_wa(req);
	}

	cs = intel_ring_begin(req, 4);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = GFX_OP_PIPE_CONTROL(4);
	*cs++ = flags;
	*cs++ = scratch_addr;
	*cs++ = 0;
	intel_ring_advance(req, cs);

	return 0;
}

static int
gen8_render_ring_flush(struct drm_i915_gem_request *req, u32 mode)
{
	u32 flags;
	u32 *cs;

	cs = intel_ring_begin(req, mode & EMIT_INVALIDATE ? 12 : 6);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	flags = PIPE_CONTROL_CS_STALL;

	if (mode & EMIT_FLUSH) {
		flags |= PIPE_CONTROL_RENDER_TARGET_CACHE_FLUSH;
		flags |= PIPE_CONTROL_DEPTH_CACHE_FLUSH;
		flags |= PIPE_CONTROL_DC_FLUSH_ENABLE;
		flags |= PIPE_CONTROL_FLUSH_ENABLE;
	}
	if (mode & EMIT_INVALIDATE) {
		flags |= PIPE_CONTROL_TLB_INVALIDATE;
		flags |= PIPE_CONTROL_INSTRUCTION_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_VF_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_CONST_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_STATE_CACHE_INVALIDATE;
		flags |= PIPE_CONTROL_QW_WRITE;
		flags |= PIPE_CONTROL_GLOBAL_GTT_IVB;

		/* WaCsStallBeforeStateCacheInvalidate:bdw,chv */
		cs = gen8_emit_pipe_control(cs,
					    PIPE_CONTROL_CS_STALL |
					    PIPE_CONTROL_STALL_AT_SCOREBOARD,
					    0);
	}

	cs = gen8_emit_pipe_control(cs, flags,
				    i915_ggtt_offset(req->engine->scratch) +
				    2 * CACHELINE_BYTES);

	intel_ring_advance(req, cs);

	return 0;
}

static void ring_setup_phys_status_page(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;
	u32 addr;

	addr = dev_priv->status_page_dmah->busaddr;
	if (INTEL_GEN(dev_priv) >= 4)
		addr |= (dev_priv->status_page_dmah->busaddr >> 28) & 0xf0;
	I915_WRITE(HWS_PGA, addr);
}

static void intel_ring_setup_status_page(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;
	i915_reg_t mmio;

	/* The ring status page addresses are no longer next to the rest of
	 * the ring registers as of gen7.
	 */
	if (IS_GEN7(dev_priv)) {
		switch (engine->id) {
		case RCS:
			mmio = RENDER_HWS_PGA_GEN7;
			break;
		case BCS:
			mmio = BLT_HWS_PGA_GEN7;
			break;
		/*
		 * VCS2 actually doesn't exist on Gen7. Only shut up
		 * gcc switch check warning
		 */
		case VCS2:
		case VCS:
			mmio = BSD_HWS_PGA_GEN7;
			break;
		case VECS:
			mmio = VEBOX_HWS_PGA_GEN7;
			break;
		}
	} else if (IS_GEN6(dev_priv)) {
		mmio = RING_HWS_PGA_GEN6(engine->mmio_base);
	} else {
		/* XXX: gen8 returns to sanity */
		mmio = RING_HWS_PGA(engine->mmio_base);
	}

	I915_WRITE(mmio, engine->status_page.ggtt_offset);
	POSTING_READ(mmio);

	/*
	 * Flush the TLB for this page
	 *
	 * FIXME: These two bits have disappeared on gen8, so a question
	 * arises: do we still need this and if so how should we go about
	 * invalidating the TLB?
	 */
	if (IS_GEN(dev_priv, 6, 7)) {
		i915_reg_t reg = RING_INSTPM(engine->mmio_base);

		/* ring should be idle before issuing a sync flush*/
		WARN_ON((I915_READ_MODE(engine) & MODE_IDLE) == 0);

		I915_WRITE(reg,
			   _MASKED_BIT_ENABLE(INSTPM_TLB_INVALIDATE |
					      INSTPM_SYNC_FLUSH));
		if (intel_wait_for_register(dev_priv,
					    reg, INSTPM_SYNC_FLUSH, 0,
					    1000))
			DRM_ERROR("%s: wait for SyncFlush to complete for TLB invalidation timed out\n",
				  engine->name);
	}
}

static bool stop_ring(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	if (INTEL_GEN(dev_priv) > 2) {
		I915_WRITE_MODE(engine, _MASKED_BIT_ENABLE(STOP_RING));
		if (intel_wait_for_register(dev_priv,
					    RING_MI_MODE(engine->mmio_base),
					    MODE_IDLE,
					    MODE_IDLE,
					    1000)) {
			DRM_ERROR("%s : timed out trying to stop ring\n",
				  engine->name);
			/* Sometimes we observe that the idle flag is not
			 * set even though the ring is empty. So double
			 * check before giving up.
			 */
			if (I915_READ_HEAD(engine) != I915_READ_TAIL(engine))
				return false;
		}
	}

	I915_WRITE_CTL(engine, 0);
	I915_WRITE_HEAD(engine, 0);
	I915_WRITE_TAIL(engine, 0);

	if (INTEL_GEN(dev_priv) > 2) {
		(void)I915_READ_CTL(engine);
		I915_WRITE_MODE(engine, _MASKED_BIT_DISABLE(STOP_RING));
	}

	return (I915_READ_HEAD(engine) & HEAD_ADDR) == 0;
}

static int init_ring_common(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;
	struct intel_ring *ring = engine->buffer;
	int ret = 0;

	intel_uncore_forcewake_get(dev_priv, FORCEWAKE_ALL);

	if (!stop_ring(engine)) {
		/* G45 ring initialization often fails to reset head to zero */
		DRM_DEBUG_KMS("%s head not reset to zero "
			      "ctl %08x head %08x tail %08x start %08x\n",
			      engine->name,
			      I915_READ_CTL(engine),
			      I915_READ_HEAD(engine),
			      I915_READ_TAIL(engine),
			      I915_READ_START(engine));

		if (!stop_ring(engine)) {
			DRM_ERROR("failed to set %s head to zero "
				  "ctl %08x head %08x tail %08x start %08x\n",
				  engine->name,
				  I915_READ_CTL(engine),
				  I915_READ_HEAD(engine),
				  I915_READ_TAIL(engine),
				  I915_READ_START(engine));
			ret = -EIO;
			goto out;
		}
	}

	if (HWS_NEEDS_PHYSICAL(dev_priv))
		ring_setup_phys_status_page(engine);
	else
		intel_ring_setup_status_page(engine);

	intel_engine_reset_breadcrumbs(engine);

	/* Enforce ordering by reading HEAD register back */
	I915_READ_HEAD(engine);

	/* Initialize the ring. This must happen _after_ we've cleared the ring
	 * registers with the above sequence (the readback of the HEAD registers
	 * also enforces ordering), otherwise the hw might lose the new ring
	 * register values. */
	I915_WRITE_START(engine, i915_ggtt_offset(ring->vma));

	/* WaClearRingBufHeadRegAtInit:ctg,elk */
	if (I915_READ_HEAD(engine))
		DRM_DEBUG("%s initialization failed [head=%08x], fudging\n",
			  engine->name, I915_READ_HEAD(engine));

	intel_ring_update_space(ring);
	I915_WRITE_HEAD(engine, ring->head);
	I915_WRITE_TAIL(engine, ring->tail);
	(void)I915_READ_TAIL(engine);

	I915_WRITE_CTL(engine, RING_CTL_SIZE(ring->size) | RING_VALID);

	/* If the head is still not zero, the ring is dead */
	if (intel_wait_for_register(dev_priv, RING_CTL(engine->mmio_base),
				    RING_VALID, RING_VALID,
				    50)) {
		DRM_ERROR("%s initialization failed "
			  "ctl %08x (valid? %d) head %08x [%08x] tail %08x [%08x] start %08x [expected %08x]\n",
			  engine->name,
			  I915_READ_CTL(engine),
			  I915_READ_CTL(engine) & RING_VALID,
			  I915_READ_HEAD(engine), ring->head,
			  I915_READ_TAIL(engine), ring->tail,
			  I915_READ_START(engine),
			  i915_ggtt_offset(ring->vma));
		ret = -EIO;
		goto out;
	}

	intel_engine_init_hangcheck(engine);

out:
	intel_uncore_forcewake_put(dev_priv, FORCEWAKE_ALL);

	return ret;
}

static void reset_ring_common(struct intel_engine_cs *engine,
			      struct drm_i915_gem_request *request)
{
	/* Try to restore the logical GPU state to match the continuation
	 * of the request queue. If we skip the context/PD restore, then
	 * the next request may try to execute assuming that its context
	 * is valid and loaded on the GPU and so may try to access invalid
	 * memory, prompting repeated GPU hangs.
	 *
	 * If the request was guilty, we still restore the logical state
	 * in case the next request requires it (e.g. the aliasing ppgtt),
	 * but skip over the hung batch.
	 *
	 * If the request was innocent, we try to replay the request with
	 * the restored context.
	 */
	if (request) {
		struct drm_i915_private *dev_priv = request->i915;
		struct intel_context *ce = &request->ctx->engine[engine->id];
		struct i915_hw_ppgtt *ppgtt;

		/* FIXME consider gen8 reset */

		if (ce->state) {
			I915_WRITE(CCID,
				   i915_ggtt_offset(ce->state) |
				   BIT(8) /* must be set! */ |
				   CCID_EXTENDED_STATE_SAVE |
				   CCID_EXTENDED_STATE_RESTORE |
				   CCID_EN);
		}

		ppgtt = request->ctx->ppgtt ?: engine->i915->mm.aliasing_ppgtt;
		if (ppgtt) {
			u32 pd_offset = ppgtt->pd.base.ggtt_offset << 10;

			I915_WRITE(RING_PP_DIR_DCLV(engine), PP_DIR_DCLV_2G);
			I915_WRITE(RING_PP_DIR_BASE(engine), pd_offset);

			/* Wait for the PD reload to complete */
			if (intel_wait_for_register(dev_priv,
						    RING_PP_DIR_BASE(engine),
						    BIT(0), 0,
						    10))
				DRM_ERROR("Wait for reload of ppgtt page-directory timed out\n");

			ppgtt->pd_dirty_rings &= ~intel_engine_flag(engine);
		}

		/* If the rq hung, jump to its breadcrumb and skip the batch */
		if (request->fence.error == -EIO)
			request->ring->head = request->postfix;
	} else {
		engine->legacy_active_context = NULL;
	}
}

static int intel_rcs_ctx_init(struct drm_i915_gem_request *req)
{
	int ret;

	ret = intel_ring_workarounds_emit(req);
	if (ret != 0)
		return ret;

	ret = i915_gem_render_state_emit(req);
	if (ret)
		return ret;

	return 0;
}

static int init_render_ring(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;
	int ret = init_ring_common(engine);
	if (ret)
		return ret;

	/* WaTimedSingleVertexDispatch:cl,bw,ctg,elk,ilk,snb */
	if (IS_GEN(dev_priv, 4, 6))
		I915_WRITE(MI_MODE, _MASKED_BIT_ENABLE(VS_TIMER_DISPATCH));

	/* We need to disable the AsyncFlip performance optimisations in order
	 * to use MI_WAIT_FOR_EVENT within the CS. It should already be
	 * programmed to '1' on all products.
	 *
	 * WaDisableAsyncFlipPerfMode:snb,ivb,hsw,vlv
	 */
	if (IS_GEN(dev_priv, 6, 7))
		I915_WRITE(MI_MODE, _MASKED_BIT_ENABLE(ASYNC_FLIP_PERF_DISABLE));

	/* Required for the hardware to program scanline values for waiting */
	/* WaEnableFlushTlbInvalidationMode:snb */
	if (IS_GEN6(dev_priv))
		I915_WRITE(GFX_MODE,
			   _MASKED_BIT_ENABLE(GFX_TLB_INVALIDATE_EXPLICIT));

	/* WaBCSVCSTlbInvalidationMode:ivb,vlv,hsw */
	if (IS_GEN7(dev_priv))
		I915_WRITE(GFX_MODE_GEN7,
			   _MASKED_BIT_ENABLE(GFX_TLB_INVALIDATE_EXPLICIT) |
			   _MASKED_BIT_ENABLE(GFX_REPLAY_MODE));

	if (IS_GEN6(dev_priv)) {
		/* From the Sandybridge PRM, volume 1 part 3, page 24:
		 * "If this bit is set, STCunit will have LRA as replacement
		 *  policy. [...] This bit must be reset.  LRA replacement
		 *  policy is not supported."
		 */
		I915_WRITE(CACHE_MODE_0,
			   _MASKED_BIT_DISABLE(CM0_STC_EVICT_DISABLE_LRA_SNB));
	}

	if (IS_GEN(dev_priv, 6, 7))
		I915_WRITE(INSTPM, _MASKED_BIT_ENABLE(INSTPM_FORCE_ORDERING));

	if (INTEL_INFO(dev_priv)->gen >= 6)
		I915_WRITE_IMR(engine, ~engine->irq_keep_mask);

	return init_workarounds_ring(engine);
}

static void render_ring_cleanup(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	i915_vma_unpin_and_release(&dev_priv->semaphore);
}

static u32 *gen8_rcs_signal(struct drm_i915_gem_request *req, u32 *cs)
{
	struct drm_i915_private *dev_priv = req->i915;
	struct intel_engine_cs *waiter;
	enum intel_engine_id id;

	for_each_engine(waiter, dev_priv, id) {
		u64 gtt_offset = req->engine->semaphore.signal_ggtt[id];
		if (gtt_offset == MI_SEMAPHORE_SYNC_INVALID)
			continue;

		*cs++ = GFX_OP_PIPE_CONTROL(6);
		*cs++ = PIPE_CONTROL_GLOBAL_GTT_IVB | PIPE_CONTROL_QW_WRITE |
			PIPE_CONTROL_CS_STALL;
		*cs++ = lower_32_bits(gtt_offset);
		*cs++ = upper_32_bits(gtt_offset);
		*cs++ = req->global_seqno;
		*cs++ = 0;
		*cs++ = MI_SEMAPHORE_SIGNAL |
			MI_SEMAPHORE_TARGET(waiter->hw_id);
		*cs++ = 0;
	}

	return cs;
}

static u32 *gen8_xcs_signal(struct drm_i915_gem_request *req, u32 *cs)
{
	struct drm_i915_private *dev_priv = req->i915;
	struct intel_engine_cs *waiter;
	enum intel_engine_id id;

	for_each_engine(waiter, dev_priv, id) {
		u64 gtt_offset = req->engine->semaphore.signal_ggtt[id];
		if (gtt_offset == MI_SEMAPHORE_SYNC_INVALID)
			continue;

		*cs++ = (MI_FLUSH_DW + 1) | MI_FLUSH_DW_OP_STOREDW;
		*cs++ = lower_32_bits(gtt_offset) | MI_FLUSH_DW_USE_GTT;
		*cs++ = upper_32_bits(gtt_offset);
		*cs++ = req->global_seqno;
		*cs++ = MI_SEMAPHORE_SIGNAL |
			MI_SEMAPHORE_TARGET(waiter->hw_id);
		*cs++ = 0;
	}

	return cs;
}

static u32 *gen6_signal(struct drm_i915_gem_request *req, u32 *cs)
{
	struct drm_i915_private *dev_priv = req->i915;
	struct intel_engine_cs *engine;
	enum intel_engine_id id;
	int num_rings = 0;

	for_each_engine(engine, dev_priv, id) {
		i915_reg_t mbox_reg;

		if (!(BIT(engine->hw_id) & GEN6_SEMAPHORES_MASK))
			continue;

		mbox_reg = req->engine->semaphore.mbox.signal[engine->hw_id];
		if (i915_mmio_reg_valid(mbox_reg)) {
			*cs++ = MI_LOAD_REGISTER_IMM(1);
			*cs++ = i915_mmio_reg_offset(mbox_reg);
			*cs++ = req->global_seqno;
			num_rings++;
		}
	}
	if (num_rings & 1)
		*cs++ = MI_NOOP;

	return cs;
}

static void i9xx_submit_request(struct drm_i915_gem_request *request)
{
	struct drm_i915_private *dev_priv = request->i915;

	i915_gem_request_submit(request);

	I915_WRITE_TAIL(request->engine,
			intel_ring_set_tail(request->ring, request->tail));
}

static void i9xx_emit_breadcrumb(struct drm_i915_gem_request *req, u32 *cs)
{
	*cs++ = MI_STORE_DWORD_INDEX;
	*cs++ = I915_GEM_HWS_INDEX << MI_STORE_DWORD_INDEX_SHIFT;
	*cs++ = req->global_seqno;
	*cs++ = MI_USER_INTERRUPT;

	req->tail = intel_ring_offset(req, cs);
	assert_ring_tail_valid(req->ring, req->tail);
}

static const int i9xx_emit_breadcrumb_sz = 4;

/**
 * gen6_sema_emit_breadcrumb - Update the semaphore mailbox registers
 *
 * @request - request to write to the ring
 *
 * Update the mailbox registers in the *other* rings with the current seqno.
 * This acts like a signal in the canonical semaphore.
 */
static void gen6_sema_emit_breadcrumb(struct drm_i915_gem_request *req, u32 *cs)
{
	return i9xx_emit_breadcrumb(req,
				    req->engine->semaphore.signal(req, cs));
}

static void gen8_render_emit_breadcrumb(struct drm_i915_gem_request *req,
					u32 *cs)
{
	struct intel_engine_cs *engine = req->engine;

	if (engine->semaphore.signal)
		cs = engine->semaphore.signal(req, cs);

	*cs++ = GFX_OP_PIPE_CONTROL(6);
	*cs++ = PIPE_CONTROL_GLOBAL_GTT_IVB | PIPE_CONTROL_CS_STALL |
		PIPE_CONTROL_QW_WRITE;
	*cs++ = intel_hws_seqno_address(engine);
	*cs++ = 0;
	*cs++ = req->global_seqno;
	/* We're thrashing one dword of HWS. */
	*cs++ = 0;
	*cs++ = MI_USER_INTERRUPT;
	*cs++ = MI_NOOP;

	req->tail = intel_ring_offset(req, cs);
	assert_ring_tail_valid(req->ring, req->tail);
}

static const int gen8_render_emit_breadcrumb_sz = 8;

/**
 * intel_ring_sync - sync the waiter to the signaller on seqno
 *
 * @waiter - ring that is waiting
 * @signaller - ring which has, or will signal
 * @seqno - seqno which the waiter will block on
 */

static int
gen8_ring_sync_to(struct drm_i915_gem_request *req,
		  struct drm_i915_gem_request *signal)
{
	struct drm_i915_private *dev_priv = req->i915;
	u64 offset = GEN8_WAIT_OFFSET(req->engine, signal->engine->id);
	struct i915_hw_ppgtt *ppgtt;
	u32 *cs;

	cs = intel_ring_begin(req, 4);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = MI_SEMAPHORE_WAIT | MI_SEMAPHORE_GLOBAL_GTT |
		MI_SEMAPHORE_SAD_GTE_SDD;
	*cs++ = signal->global_seqno;
	*cs++ = lower_32_bits(offset);
	*cs++ = upper_32_bits(offset);
	intel_ring_advance(req, cs);

	/* When the !RCS engines idle waiting upon a semaphore, they lose their
	 * pagetables and we must reload them before executing the batch.
	 * We do this on the i915_switch_context() following the wait and
	 * before the dispatch.
	 */
	ppgtt = req->ctx->ppgtt;
	if (ppgtt && req->engine->id != RCS)
		ppgtt->pd_dirty_rings |= intel_engine_flag(req->engine);
	return 0;
}

static int
gen6_ring_sync_to(struct drm_i915_gem_request *req,
		  struct drm_i915_gem_request *signal)
{
	u32 dw1 = MI_SEMAPHORE_MBOX |
		  MI_SEMAPHORE_COMPARE |
		  MI_SEMAPHORE_REGISTER;
	u32 wait_mbox = signal->engine->semaphore.mbox.wait[req->engine->hw_id];
	u32 *cs;

	WARN_ON(wait_mbox == MI_SEMAPHORE_SYNC_INVALID);

	cs = intel_ring_begin(req, 4);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = dw1 | wait_mbox;
	/* Throughout all of the GEM code, seqno passed implies our current
	 * seqno is >= the last seqno executed. However for hardware the
	 * comparison is strictly greater than.
	 */
	*cs++ = signal->global_seqno - 1;
	*cs++ = 0;
	*cs++ = MI_NOOP;
	intel_ring_advance(req, cs);

	return 0;
}

static void
gen5_seqno_barrier(struct intel_engine_cs *engine)
{
	/* MI_STORE are internally buffered by the GPU and not flushed
	 * either by MI_FLUSH or SyncFlush or any other combination of
	 * MI commands.
	 *
	 * "Only the submission of the store operation is guaranteed.
	 * The write result will be complete (coherent) some time later
	 * (this is practically a finite period but there is no guaranteed
	 * latency)."
	 *
	 * Empirically, we observe that we need a delay of at least 75us to
	 * be sure that the seqno write is visible by the CPU.
	 */
	usleep_range(125, 250);
}

static void
gen6_seqno_barrier(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	/* Workaround to force correct ordering between irq and seqno writes on
	 * ivb (and maybe also on snb) by reading from a CS register (like
	 * ACTHD) before reading the status page.
	 *
	 * Note that this effectively stalls the read by the time it takes to
	 * do a memory transaction, which more or less ensures that the write
	 * from the GPU has sufficient time to invalidate the CPU cacheline.
	 * Alternatively we could delay the interrupt from the CS ring to give
	 * the write time to land, but that would incur a delay after every
	 * batch i.e. much more frequent than a delay when waiting for the
	 * interrupt (with the same net latency).
	 *
	 * Also note that to prevent whole machine hangs on gen7, we have to
	 * take the spinlock to guard against concurrent cacheline access.
	 */
	spin_lock_irq(&dev_priv->uncore.lock);
	POSTING_READ_FW(RING_ACTHD(engine->mmio_base));
	spin_unlock_irq(&dev_priv->uncore.lock);
}

static void
gen5_irq_enable(struct intel_engine_cs *engine)
{
	gen5_enable_gt_irq(engine->i915, engine->irq_enable_mask);
}

static void
gen5_irq_disable(struct intel_engine_cs *engine)
{
	gen5_disable_gt_irq(engine->i915, engine->irq_enable_mask);
}

static void
i9xx_irq_enable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	dev_priv->irq_mask &= ~engine->irq_enable_mask;
	I915_WRITE(IMR, dev_priv->irq_mask);
	POSTING_READ_FW(RING_IMR(engine->mmio_base));
}

static void
i9xx_irq_disable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	dev_priv->irq_mask |= engine->irq_enable_mask;
	I915_WRITE(IMR, dev_priv->irq_mask);
}

static void
i8xx_irq_enable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	dev_priv->irq_mask &= ~engine->irq_enable_mask;
	I915_WRITE16(IMR, dev_priv->irq_mask);
	POSTING_READ16(RING_IMR(engine->mmio_base));
}

static void
i8xx_irq_disable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	dev_priv->irq_mask |= engine->irq_enable_mask;
	I915_WRITE16(IMR, dev_priv->irq_mask);
}

static int
bsd_ring_flush(struct drm_i915_gem_request *req, u32 mode)
{
	u32 *cs;

	cs = intel_ring_begin(req, 2);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = MI_FLUSH;
	*cs++ = MI_NOOP;
	intel_ring_advance(req, cs);
	return 0;
}

static void
gen6_irq_enable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	I915_WRITE_IMR(engine,
		       ~(engine->irq_enable_mask |
			 engine->irq_keep_mask));
	gen5_enable_gt_irq(dev_priv, engine->irq_enable_mask);
}

static void
gen6_irq_disable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	I915_WRITE_IMR(engine, ~engine->irq_keep_mask);
	gen5_disable_gt_irq(dev_priv, engine->irq_enable_mask);
}

static void
hsw_vebox_irq_enable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	I915_WRITE_IMR(engine, ~engine->irq_enable_mask);
	gen6_unmask_pm_irq(dev_priv, engine->irq_enable_mask);
}

static void
hsw_vebox_irq_disable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	I915_WRITE_IMR(engine, ~0);
	gen6_mask_pm_irq(dev_priv, engine->irq_enable_mask);
}

static void
gen8_irq_enable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	I915_WRITE_IMR(engine,
		       ~(engine->irq_enable_mask |
			 engine->irq_keep_mask));
	POSTING_READ_FW(RING_IMR(engine->mmio_base));
}

static void
gen8_irq_disable(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	I915_WRITE_IMR(engine, ~engine->irq_keep_mask);
}

static int
i965_emit_bb_start(struct drm_i915_gem_request *req,
		   u64 offset, u32 length,
		   unsigned int dispatch_flags)
{
	u32 *cs;

	cs = intel_ring_begin(req, 2);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = MI_BATCH_BUFFER_START | MI_BATCH_GTT | (dispatch_flags &
		I915_DISPATCH_SECURE ? 0 : MI_BATCH_NON_SECURE_I965);
	*cs++ = offset;
	intel_ring_advance(req, cs);

	return 0;
}

/* Just userspace ABI convention to limit the wa batch bo to a resonable size */
#define I830_BATCH_LIMIT (256*1024)
#define I830_TLB_ENTRIES (2)
#define I830_WA_SIZE max(I830_TLB_ENTRIES*4096, I830_BATCH_LIMIT)
static int
i830_emit_bb_start(struct drm_i915_gem_request *req,
		   u64 offset, u32 len,
		   unsigned int dispatch_flags)
{
	u32 *cs, cs_offset = i915_ggtt_offset(req->engine->scratch);

	cs = intel_ring_begin(req, 6);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	/* Evict the invalid PTE TLBs */
	*cs++ = COLOR_BLT_CMD | BLT_WRITE_RGBA;
	*cs++ = BLT_DEPTH_32 | BLT_ROP_COLOR_COPY | 4096;
	*cs++ = I830_TLB_ENTRIES << 16 | 4; /* load each page */
	*cs++ = cs_offset;
	*cs++ = 0xdeadbeef;
	*cs++ = MI_NOOP;
	intel_ring_advance(req, cs);

	if ((dispatch_flags & I915_DISPATCH_PINNED) == 0) {
		if (len > I830_BATCH_LIMIT)
			return -ENOSPC;

		cs = intel_ring_begin(req, 6 + 2);
		if (IS_ERR(cs))
			return PTR_ERR(cs);

		/* Blit the batch (which has now all relocs applied) to the
		 * stable batch scratch bo area (so that the CS never
		 * stumbles over its tlb invalidation bug) ...
		 */
		*cs++ = SRC_COPY_BLT_CMD | BLT_WRITE_RGBA;
		*cs++ = BLT_DEPTH_32 | BLT_ROP_SRC_COPY | 4096;
		*cs++ = DIV_ROUND_UP(len, 4096) << 16 | 4096;
		*cs++ = cs_offset;
		*cs++ = 4096;
		*cs++ = offset;

		*cs++ = MI_FLUSH;
		*cs++ = MI_NOOP;
		intel_ring_advance(req, cs);

		/* ... and execute it. */
		offset = cs_offset;
	}

	cs = intel_ring_begin(req, 2);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = MI_BATCH_BUFFER_START | MI_BATCH_GTT;
	*cs++ = offset | (dispatch_flags & I915_DISPATCH_SECURE ? 0 :
		MI_BATCH_NON_SECURE);
	intel_ring_advance(req, cs);

	return 0;
}

static int
i915_emit_bb_start(struct drm_i915_gem_request *req,
		   u64 offset, u32 len,
		   unsigned int dispatch_flags)
{
	u32 *cs;

	cs = intel_ring_begin(req, 2);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = MI_BATCH_BUFFER_START | MI_BATCH_GTT;
	*cs++ = offset | (dispatch_flags & I915_DISPATCH_SECURE ? 0 :
		MI_BATCH_NON_SECURE);
	intel_ring_advance(req, cs);

	return 0;
}

static void cleanup_phys_status_page(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	if (!dev_priv->status_page_dmah)
		return;

	drm_pci_free(&dev_priv->drm, dev_priv->status_page_dmah);
	engine->status_page.page_addr = NULL;
}

static void cleanup_status_page(struct intel_engine_cs *engine)
{
	struct i915_vma *vma;
	struct drm_i915_gem_object *obj;

	vma = fetch_and_zero(&engine->status_page.vma);
	if (!vma)
		return;

	obj = vma->obj;

	i915_vma_unpin(vma);
	i915_vma_close(vma);

	i915_gem_object_unpin_map(obj);
	__i915_gem_object_release_unless_active(obj);
}

static int init_status_page(struct intel_engine_cs *engine)
{
	struct drm_i915_gem_object *obj;
	struct i915_vma *vma;
	unsigned int flags;
	void *vaddr;
	int ret;

	obj = i915_gem_object_create_internal(engine->i915, PAGE_SIZE);
	if (IS_ERR(obj)) {
		DRM_ERROR("Failed to allocate status page\n");
		return PTR_ERR(obj);
	}

	ret = i915_gem_object_set_cache_level(obj, I915_CACHE_LLC);
	if (ret)
		goto err;

	vma = i915_vma_instance(obj, &engine->i915->ggtt.base, NULL);
	if (IS_ERR(vma)) {
		ret = PTR_ERR(vma);
		goto err;
	}

	flags = PIN_GLOBAL;
	if (!HAS_LLC(engine->i915))
		/* On g33, we cannot place HWS above 256MiB, so
		 * restrict its pinning to the low mappable arena.
		 * Though this restriction is not documented for
		 * gen4, gen5, or byt, they also behave similarly
		 * and hang if the HWS is placed at the top of the
		 * GTT. To generalise, it appears that all !llc
		 * platforms have issues with us placing the HWS
		 * above the mappable region (even though we never
		 * actualy map it).
		 */
		flags |= PIN_MAPPABLE;
	ret = i915_vma_pin(vma, 0, 4096, flags);
	if (ret)
		goto err;

	vaddr = i915_gem_object_pin_map(obj, I915_MAP_WB);
	if (IS_ERR(vaddr)) {
		ret = PTR_ERR(vaddr);
		goto err_unpin;
	}

	engine->status_page.vma = vma;
	engine->status_page.ggtt_offset = i915_ggtt_offset(vma);
	engine->status_page.page_addr = memset(vaddr, 0, PAGE_SIZE);

	DRM_DEBUG_DRIVER("%s hws offset: 0x%08x\n",
			 engine->name, i915_ggtt_offset(vma));
	return 0;

err_unpin:
	i915_vma_unpin(vma);
err:
	i915_gem_object_put(obj);
	return ret;
}

static int init_phys_status_page(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	GEM_BUG_ON(engine->id != RCS);

	dev_priv->status_page_dmah =
		drm_pci_alloc(&dev_priv->drm, PAGE_SIZE, PAGE_SIZE);
	if (!dev_priv->status_page_dmah)
		return -ENOMEM;

	engine->status_page.page_addr = dev_priv->status_page_dmah->vaddr;
	memset(engine->status_page.page_addr, 0, PAGE_SIZE);

	return 0;
}

int intel_ring_pin(struct intel_ring *ring,
		   struct drm_i915_private *i915,
		   unsigned int offset_bias)
{
	enum i915_map_type map = HAS_LLC(i915) ? I915_MAP_WB : I915_MAP_WC;
	struct i915_vma *vma = ring->vma;
	unsigned int flags;
	void *addr;
	int ret;

	GEM_BUG_ON(ring->vaddr);


	flags = PIN_GLOBAL;
	if (offset_bias)
		flags |= PIN_OFFSET_BIAS | offset_bias;
	if (vma->obj->stolen)
		flags |= PIN_MAPPABLE;

	if (!(vma->flags & I915_VMA_GLOBAL_BIND)) {
		if (flags & PIN_MAPPABLE || map == I915_MAP_WC)
			ret = i915_gem_object_set_to_gtt_domain(vma->obj, true);
		else
			ret = i915_gem_object_set_to_cpu_domain(vma->obj, true);
		if (unlikely(ret))
			return ret;
	}

	ret = i915_vma_pin(vma, 0, PAGE_SIZE, flags);
	if (unlikely(ret))
		return ret;

	if (i915_vma_is_map_and_fenceable(vma))
		addr = (void __force *)i915_vma_pin_iomap(vma);
	else
		addr = i915_gem_object_pin_map(vma->obj, map);
	if (IS_ERR(addr))
		goto err;

	ring->vaddr = addr;
	return 0;

err:
	i915_vma_unpin(vma);
	return PTR_ERR(addr);
}

void intel_ring_reset(struct intel_ring *ring, u32 tail)
{
	GEM_BUG_ON(!list_empty(&ring->request_list));
	ring->tail = tail;
	ring->head = tail;
	ring->emit = tail;
	intel_ring_update_space(ring);
}

void intel_ring_unpin(struct intel_ring *ring)
{
	GEM_BUG_ON(!ring->vma);
	GEM_BUG_ON(!ring->vaddr);

	/* Discard any unused bytes beyond that submitted to hw. */
	intel_ring_reset(ring, ring->tail);

	if (i915_vma_is_map_and_fenceable(ring->vma))
		i915_vma_unpin_iomap(ring->vma);
	else
		i915_gem_object_unpin_map(ring->vma->obj);
	ring->vaddr = NULL;

	i915_vma_unpin(ring->vma);
}

static struct i915_vma *
intel_ring_create_vma(struct drm_i915_private *dev_priv, int size)
{
	struct drm_i915_gem_object *obj;
	struct i915_vma *vma;

	obj = i915_gem_object_create_stolen(dev_priv, size);
	if (!obj)
		obj = i915_gem_object_create_internal(dev_priv, size);
	if (IS_ERR(obj))
		return ERR_CAST(obj);

	/* mark ring buffers as read-only from GPU side by default */
	obj->gt_ro = 1;

	vma = i915_vma_instance(obj, &dev_priv->ggtt.base, NULL);
	if (IS_ERR(vma))
		goto err;

	return vma;

err:
	i915_gem_object_put(obj);
	return vma;
}

struct intel_ring *
intel_engine_create_ring(struct intel_engine_cs *engine, int size)
{
	struct intel_ring *ring;
	struct i915_vma *vma;

	GEM_BUG_ON(!is_power_of_2(size));
	GEM_BUG_ON(RING_CTL_SIZE(size) & ~RING_NR_PAGES);

	ring = kzalloc(sizeof(*ring), GFP_KERNEL);
	if (!ring)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&ring->request_list);

	ring->size = size;
	/* Workaround an erratum on the i830 which causes a hang if
	 * the TAIL pointer points to within the last 2 cachelines
	 * of the buffer.
	 */
	ring->effective_size = size;
	if (IS_I830(engine->i915) || IS_I845G(engine->i915))
		ring->effective_size -= 2 * CACHELINE_BYTES;

	intel_ring_update_space(ring);

	vma = intel_ring_create_vma(engine->i915, size);
	if (IS_ERR(vma)) {
		kfree(ring);
		return ERR_CAST(vma);
	}
	ring->vma = vma;

	return ring;
}

void
intel_ring_free(struct intel_ring *ring)
{
	struct drm_i915_gem_object *obj = ring->vma->obj;

	i915_vma_close(ring->vma);
	__i915_gem_object_release_unless_active(obj);

	kfree(ring);
}

static int context_pin(struct i915_gem_context *ctx)
{
	struct i915_vma *vma = ctx->engine[RCS].state;
	int ret;

	/* Clear this page out of any CPU caches for coherent swap-in/out.
	 * We only want to do this on the first bind so that we do not stall
	 * on an active context (which by nature is already on the GPU).
	 */
	if (!(vma->flags & I915_VMA_GLOBAL_BIND)) {
		ret = i915_gem_object_set_to_gtt_domain(vma->obj, false);
		if (ret)
			return ret;
	}

	return i915_vma_pin(vma, 0, I915_GTT_MIN_ALIGNMENT,
			    PIN_GLOBAL | PIN_HIGH);
}

static struct i915_vma *
alloc_context_vma(struct intel_engine_cs *engine)
{
	struct drm_i915_private *i915 = engine->i915;
	struct drm_i915_gem_object *obj;
	struct i915_vma *vma;

	obj = i915_gem_object_create(i915, i915->hw_context_size);
	if (IS_ERR(obj))
		return ERR_CAST(obj);

	/*
	 * Try to make the context utilize L3 as well as LLC.
	 *
	 * On VLV we don't have L3 controls in the PTEs so we
	 * shouldn't touch the cache level, especially as that
	 * would make the object snooped which might have a
	 * negative performance impact.
	 *
	 * Snooping is required on non-llc platforms in execlist
	 * mode, but since all GGTT accesses use PAT entry 0 we
	 * get snooping anyway regardless of cache_level.
	 *
	 * This is only applicable for Ivy Bridge devices since
	 * later platforms don't have L3 control bits in the PTE.
	 */
	if (IS_IVYBRIDGE(i915)) {
		/* Ignore any error, regard it as a simple optimisation */
		i915_gem_object_set_cache_level(obj, I915_CACHE_L3_LLC);
	}

	vma = i915_vma_instance(obj, &i915->ggtt.base, NULL);
	if (IS_ERR(vma))
		i915_gem_object_put(obj);

	return vma;
}

static int intel_ring_context_pin(struct intel_engine_cs *engine,
				  struct i915_gem_context *ctx)
{
	struct intel_context *ce = &ctx->engine[engine->id];
	int ret;

	lockdep_assert_held(&ctx->i915->drm.struct_mutex);

	if (ce->pin_count++)
		return 0;
	GEM_BUG_ON(!ce->pin_count); /* no overflow please! */

	if (engine->id == RCS && !ce->state && engine->i915->hw_context_size) {
		struct i915_vma *vma;

		vma = alloc_context_vma(engine);
		if (IS_ERR(vma)) {
			ret = PTR_ERR(vma);
			goto error;
		}

		ce->state = vma;
	}

	if (ce->state) {
		ret = context_pin(ctx);
		if (ret)
			goto error;

		ce->state->obj->mm.dirty = true;
	}

	/* The kernel context is only used as a placeholder for flushing the
	 * active context. It is never used for submitting user rendering and
	 * as such never requires the golden render context, and so we can skip
	 * emitting it when we switch to the kernel context. This is required
	 * as during eviction we cannot allocate and pin the renderstate in
	 * order to initialise the context.
	 */
	if (i915_gem_context_is_kernel(ctx))
		ce->initialised = true;

	i915_gem_context_get(ctx);
	return 0;

error:
	ce->pin_count = 0;
	return ret;
}

static void intel_ring_context_unpin(struct intel_engine_cs *engine,
				     struct i915_gem_context *ctx)
{
	struct intel_context *ce = &ctx->engine[engine->id];

	lockdep_assert_held(&ctx->i915->drm.struct_mutex);
	GEM_BUG_ON(ce->pin_count == 0);

	if (--ce->pin_count)
		return;

	if (ce->state)
		i915_vma_unpin(ce->state);

	i915_gem_context_put(ctx);
}

static int intel_init_ring_buffer(struct intel_engine_cs *engine)
{
	struct intel_ring *ring;
	int err;

	intel_engine_setup_common(engine);

	err = intel_engine_init_common(engine);
	if (err)
		goto err;

	if (HWS_NEEDS_PHYSICAL(engine->i915))
		err = init_phys_status_page(engine);
	else
		err = init_status_page(engine);
	if (err)
		goto err;

	ring = intel_engine_create_ring(engine, 32 * PAGE_SIZE);
	if (IS_ERR(ring)) {
		err = PTR_ERR(ring);
		goto err_hws;
	}

	/* Ring wraparound at offset 0 sometimes hangs. No idea why. */
	err = intel_ring_pin(ring, engine->i915, I915_GTT_PAGE_SIZE);
	if (err)
		goto err_ring;

	GEM_BUG_ON(engine->buffer);
	engine->buffer = ring;

	return 0;

err_ring:
	intel_ring_free(ring);
err_hws:
	if (HWS_NEEDS_PHYSICAL(engine->i915))
		cleanup_phys_status_page(engine);
	else
		cleanup_status_page(engine);
err:
	intel_engine_cleanup_common(engine);
	return err;
}

void intel_engine_cleanup(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	WARN_ON(INTEL_GEN(dev_priv) > 2 &&
		(I915_READ_MODE(engine) & MODE_IDLE) == 0);

	intel_ring_unpin(engine->buffer);
	intel_ring_free(engine->buffer);

	if (engine->cleanup)
		engine->cleanup(engine);

	if (HWS_NEEDS_PHYSICAL(dev_priv))
		cleanup_phys_status_page(engine);
	else
		cleanup_status_page(engine);

	intel_engine_cleanup_common(engine);

	dev_priv->engine[engine->id] = NULL;
	kfree(engine);
}

void intel_legacy_submission_resume(struct drm_i915_private *dev_priv)
{
	struct intel_engine_cs *engine;
	enum intel_engine_id id;

	/* Restart from the beginning of the rings for convenience */
	for_each_engine(engine, dev_priv, id)
		intel_ring_reset(engine->buffer, 0);
}

static int ring_request_alloc(struct drm_i915_gem_request *request)
{
	u32 *cs;

	GEM_BUG_ON(!request->ctx->engine[request->engine->id].pin_count);

	/* Flush enough space to reduce the likelihood of waiting after
	 * we start building the request - in which case we will just
	 * have to repeat work.
	 */
	request->reserved_space += LEGACY_REQUEST_SIZE;

	GEM_BUG_ON(!request->engine->buffer);
	request->ring = request->engine->buffer;

	cs = intel_ring_begin(request, 0);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	request->reserved_space -= LEGACY_REQUEST_SIZE;
	return 0;
}

static int wait_for_space(struct drm_i915_gem_request *req, int bytes)
{
	struct intel_ring *ring = req->ring;
	struct drm_i915_gem_request *target;
	long timeout;

	lockdep_assert_held(&req->i915->drm.struct_mutex);

	intel_ring_update_space(ring);
	if (ring->space >= bytes)
		return 0;

	/*
	 * Space is reserved in the ringbuffer for finalising the request,
	 * as that cannot be allowed to fail. During request finalisation,
	 * reserved_space is set to 0 to stop the overallocation and the
	 * assumption is that then we never need to wait (which has the
	 * risk of failing with EINTR).
	 *
	 * See also i915_gem_request_alloc() and i915_add_request().
	 */
	GEM_BUG_ON(!req->reserved_space);

	list_for_each_entry(target, &ring->request_list, ring_link) {
		unsigned space;

		/* Would completion of this request free enough space? */
		space = __intel_ring_space(target->postfix, ring->emit,
					   ring->size);
		if (space >= bytes)
			break;
	}

	if (WARN_ON(&target->ring_link == &ring->request_list))
		return -ENOSPC;

	timeout = i915_wait_request(target,
				    I915_WAIT_INTERRUPTIBLE | I915_WAIT_LOCKED,
				    MAX_SCHEDULE_TIMEOUT);
	if (timeout < 0)
		return timeout;

	i915_gem_request_retire_upto(target);

	intel_ring_update_space(ring);
	GEM_BUG_ON(ring->space < bytes);
	return 0;
}

u32 *intel_ring_begin(struct drm_i915_gem_request *req, int num_dwords)
{
	struct intel_ring *ring = req->ring;
	int remain_actual = ring->size - ring->emit;
	int remain_usable = ring->effective_size - ring->emit;
	int bytes = num_dwords * sizeof(u32);
	int total_bytes, wait_bytes;
	bool need_wrap = false;
	u32 *cs;

	total_bytes = bytes + req->reserved_space;

	if (unlikely(bytes > remain_usable)) {
		/*
		 * Not enough space for the basic request. So need to flush
		 * out the remainder and then wait for base + reserved.
		 */
		wait_bytes = remain_actual + total_bytes;
		need_wrap = true;
	} else if (unlikely(total_bytes > remain_usable)) {
		/*
		 * The base request will fit but the reserved space
		 * falls off the end. So we don't need an immediate wrap
		 * and only need to effectively wait for the reserved
		 * size space from the start of ringbuffer.
		 */
		wait_bytes = remain_actual + req->reserved_space;
	} else {
		/* No wrapping required, just waiting. */
		wait_bytes = total_bytes;
	}

	if (wait_bytes > ring->space) {
		int ret = wait_for_space(req, wait_bytes);
		if (unlikely(ret))
			return ERR_PTR(ret);
	}

	if (unlikely(need_wrap)) {
		GEM_BUG_ON(remain_actual > ring->space);
		GEM_BUG_ON(ring->emit + remain_actual > ring->size);

		/* Fill the tail with MI_NOOP */
		memset(ring->vaddr + ring->emit, 0, remain_actual);
		ring->emit = 0;
		ring->space -= remain_actual;
	}

	GEM_BUG_ON(ring->emit > ring->size - bytes);
	cs = ring->vaddr + ring->emit;
	GEM_DEBUG_EXEC(memset(cs, POISON_INUSE, bytes));
	ring->emit += bytes;
	ring->space -= bytes;
	GEM_BUG_ON(ring->space < 0);

	return cs;
}

/* Align the ring tail to a cacheline boundary */
int intel_ring_cacheline_align(struct drm_i915_gem_request *req)
{
	int num_dwords =
		(req->ring->emit & (CACHELINE_BYTES - 1)) / sizeof(uint32_t);
	u32 *cs;

	if (num_dwords == 0)
		return 0;

	num_dwords = CACHELINE_BYTES / sizeof(uint32_t) - num_dwords;
	cs = intel_ring_begin(req, num_dwords);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	while (num_dwords--)
		*cs++ = MI_NOOP;

	intel_ring_advance(req, cs);

	return 0;
}

static void gen6_bsd_submit_request(struct drm_i915_gem_request *request)
{
	struct drm_i915_private *dev_priv = request->i915;

	intel_uncore_forcewake_get(dev_priv, FORCEWAKE_ALL);

       /* Every tail move must follow the sequence below */

	/* Disable notification that the ring is IDLE. The GT
	 * will then assume that it is busy and bring it out of rc6.
	 */
	I915_WRITE_FW(GEN6_BSD_SLEEP_PSMI_CONTROL,
		      _MASKED_BIT_ENABLE(GEN6_BSD_SLEEP_MSG_DISABLE));

	/* Clear the context id. Here be magic! */
	I915_WRITE64_FW(GEN6_BSD_RNCID, 0x0);

	/* Wait for the ring not to be idle, i.e. for it to wake up. */
	if (__intel_wait_for_register_fw(dev_priv,
					 GEN6_BSD_SLEEP_PSMI_CONTROL,
					 GEN6_BSD_SLEEP_INDICATOR,
					 0,
					 1000, 0, NULL))
		DRM_ERROR("timed out waiting for the BSD ring to wake up\n");

	/* Now that the ring is fully powered up, update the tail */
	i9xx_submit_request(request);

	/* Let the ring send IDLE messages to the GT again,
	 * and so let it sleep to conserve power when idle.
	 */
	I915_WRITE_FW(GEN6_BSD_SLEEP_PSMI_CONTROL,
		      _MASKED_BIT_DISABLE(GEN6_BSD_SLEEP_MSG_DISABLE));

	intel_uncore_forcewake_put(dev_priv, FORCEWAKE_ALL);
}

static int gen6_bsd_ring_flush(struct drm_i915_gem_request *req, u32 mode)
{
	u32 cmd, *cs;

	cs = intel_ring_begin(req, 4);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	cmd = MI_FLUSH_DW;
	if (INTEL_GEN(req->i915) >= 8)
		cmd += 1;

	/* We always require a command barrier so that subsequent
	 * commands, such as breadcrumb interrupts, are strictly ordered
	 * wrt the contents of the write cache being flushed to memory
	 * (and thus being coherent from the CPU).
	 */
	cmd |= MI_FLUSH_DW_STORE_INDEX | MI_FLUSH_DW_OP_STOREDW;

	/*
	 * Bspec vol 1c.5 - video engine command streamer:
	 * "If ENABLED, all TLBs will be invalidated once the flush
	 * operation is complete. This bit is only valid when the
	 * Post-Sync Operation field is a value of 1h or 3h."
	 */
	if (mode & EMIT_INVALIDATE)
		cmd |= MI_INVALIDATE_TLB | MI_INVALIDATE_BSD;

	*cs++ = cmd;
	*cs++ = I915_GEM_HWS_SCRATCH_ADDR | MI_FLUSH_DW_USE_GTT;
	if (INTEL_GEN(req->i915) >= 8) {
		*cs++ = 0; /* upper addr */
		*cs++ = 0; /* value */
	} else  {
		*cs++ = 0;
		*cs++ = MI_NOOP;
	}
	intel_ring_advance(req, cs);
	return 0;
}

static int
gen8_emit_bb_start(struct drm_i915_gem_request *req,
		   u64 offset, u32 len,
		   unsigned int dispatch_flags)
{
	bool ppgtt = USES_PPGTT(req->i915) &&
			!(dispatch_flags & I915_DISPATCH_SECURE);
	u32 *cs;

	cs = intel_ring_begin(req, 4);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	/* FIXME(BDW): Address space and security selectors. */
	*cs++ = MI_BATCH_BUFFER_START_GEN8 | (ppgtt << 8) | (dispatch_flags &
		I915_DISPATCH_RS ? MI_BATCH_RESOURCE_STREAMER : 0);
	*cs++ = lower_32_bits(offset);
	*cs++ = upper_32_bits(offset);
	*cs++ = MI_NOOP;
	intel_ring_advance(req, cs);

	return 0;
}

static int
hsw_emit_bb_start(struct drm_i915_gem_request *req,
		  u64 offset, u32 len,
		  unsigned int dispatch_flags)
{
	u32 *cs;

	cs = intel_ring_begin(req, 2);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = MI_BATCH_BUFFER_START | (dispatch_flags & I915_DISPATCH_SECURE ?
		0 : MI_BATCH_PPGTT_HSW | MI_BATCH_NON_SECURE_HSW) |
		(dispatch_flags & I915_DISPATCH_RS ?
		MI_BATCH_RESOURCE_STREAMER : 0);
	/* bit0-7 is the length on GEN6+ */
	*cs++ = offset;
	intel_ring_advance(req, cs);

	return 0;
}

static int
gen6_emit_bb_start(struct drm_i915_gem_request *req,
		   u64 offset, u32 len,
		   unsigned int dispatch_flags)
{
	u32 *cs;

	cs = intel_ring_begin(req, 2);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	*cs++ = MI_BATCH_BUFFER_START | (dispatch_flags & I915_DISPATCH_SECURE ?
		0 : MI_BATCH_NON_SECURE_I965);
	/* bit0-7 is the length on GEN6+ */
	*cs++ = offset;
	intel_ring_advance(req, cs);

	return 0;
}

/* Blitter support (SandyBridge+) */

static int gen6_ring_flush(struct drm_i915_gem_request *req, u32 mode)
{
	u32 cmd, *cs;

	cs = intel_ring_begin(req, 4);
	if (IS_ERR(cs))
		return PTR_ERR(cs);

	cmd = MI_FLUSH_DW;
	if (INTEL_GEN(req->i915) >= 8)
		cmd += 1;

	/* We always require a command barrier so that subsequent
	 * commands, such as breadcrumb interrupts, are strictly ordered
	 * wrt the contents of the write cache being flushed to memory
	 * (and thus being coherent from the CPU).
	 */
	cmd |= MI_FLUSH_DW_STORE_INDEX | MI_FLUSH_DW_OP_STOREDW;

	/*
	 * Bspec vol 1c.3 - blitter engine command streamer:
	 * "If ENABLED, all TLBs will be invalidated once the flush
	 * operation is complete. This bit is only valid when the
	 * Post-Sync Operation field is a value of 1h or 3h."
	 */
	if (mode & EMIT_INVALIDATE)
		cmd |= MI_INVALIDATE_TLB;
	*cs++ = cmd;
	*cs++ = I915_GEM_HWS_SCRATCH_ADDR | MI_FLUSH_DW_USE_GTT;
	if (INTEL_GEN(req->i915) >= 8) {
		*cs++ = 0; /* upper addr */
		*cs++ = 0; /* value */
	} else  {
		*cs++ = 0;
		*cs++ = MI_NOOP;
	}
	intel_ring_advance(req, cs);

	return 0;
}

static void intel_ring_init_semaphores(struct drm_i915_private *dev_priv,
				       struct intel_engine_cs *engine)
{
	struct drm_i915_gem_object *obj;
	int ret, i;

	if (!i915.semaphores)
		return;

	if (INTEL_GEN(dev_priv) >= 8 && !dev_priv->semaphore) {
		struct i915_vma *vma;

		obj = i915_gem_object_create(dev_priv, PAGE_SIZE);
		if (IS_ERR(obj))
			goto err;

		vma = i915_vma_instance(obj, &dev_priv->ggtt.base, NULL);
		if (IS_ERR(vma))
			goto err_obj;

		ret = i915_gem_object_set_to_gtt_domain(obj, false);
		if (ret)
			goto err_obj;

		ret = i915_vma_pin(vma, 0, 0, PIN_GLOBAL | PIN_HIGH);
		if (ret)
			goto err_obj;

		dev_priv->semaphore = vma;
	}

	if (INTEL_GEN(dev_priv) >= 8) {
		u32 offset = i915_ggtt_offset(dev_priv->semaphore);

		engine->semaphore.sync_to = gen8_ring_sync_to;
		engine->semaphore.signal = gen8_xcs_signal;

		for (i = 0; i < I915_NUM_ENGINES; i++) {
			u32 ring_offset;

			if (i != engine->id)
				ring_offset = offset + GEN8_SEMAPHORE_OFFSET(engine->id, i);
			else
				ring_offset = MI_SEMAPHORE_SYNC_INVALID;

			engine->semaphore.signal_ggtt[i] = ring_offset;
		}
	} else if (INTEL_GEN(dev_priv) >= 6) {
		engine->semaphore.sync_to = gen6_ring_sync_to;
		engine->semaphore.signal = gen6_signal;

		/*
		 * The current semaphore is only applied on pre-gen8
		 * platform.  And there is no VCS2 ring on the pre-gen8
		 * platform. So the semaphore between RCS and VCS2 is
		 * initialized as INVALID.  Gen8 will initialize the
		 * sema between VCS2 and RCS later.
		 */
		for (i = 0; i < GEN6_NUM_SEMAPHORES; i++) {
			static const struct {
				u32 wait_mbox;
				i915_reg_t mbox_reg;
			} sem_data[GEN6_NUM_SEMAPHORES][GEN6_NUM_SEMAPHORES] = {
				[RCS_HW] = {
					[VCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_RV,  .mbox_reg = GEN6_VRSYNC },
					[BCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_RB,  .mbox_reg = GEN6_BRSYNC },
					[VECS_HW] = { .wait_mbox = MI_SEMAPHORE_SYNC_RVE, .mbox_reg = GEN6_VERSYNC },
				},
				[VCS_HW] = {
					[RCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_VR,  .mbox_reg = GEN6_RVSYNC },
					[BCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_VB,  .mbox_reg = GEN6_BVSYNC },
					[VECS_HW] = { .wait_mbox = MI_SEMAPHORE_SYNC_VVE, .mbox_reg = GEN6_VEVSYNC },
				},
				[BCS_HW] = {
					[RCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_BR,  .mbox_reg = GEN6_RBSYNC },
					[VCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_BV,  .mbox_reg = GEN6_VBSYNC },
					[VECS_HW] = { .wait_mbox = MI_SEMAPHORE_SYNC_BVE, .mbox_reg = GEN6_VEBSYNC },
				},
				[VECS_HW] = {
					[RCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_VER, .mbox_reg = GEN6_RVESYNC },
					[VCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_VEV, .mbox_reg = GEN6_VVESYNC },
					[BCS_HW] =  { .wait_mbox = MI_SEMAPHORE_SYNC_VEB, .mbox_reg = GEN6_BVESYNC },
				},
			};
			u32 wait_mbox;
			i915_reg_t mbox_reg;

			if (i == engine->hw_id) {
				wait_mbox = MI_SEMAPHORE_SYNC_INVALID;
				mbox_reg = GEN6_NOSYNC;
			} else {
				wait_mbox = sem_data[engine->hw_id][i].wait_mbox;
				mbox_reg = sem_data[engine->hw_id][i].mbox_reg;
			}

			engine->semaphore.mbox.wait[i] = wait_mbox;
			engine->semaphore.mbox.signal[i] = mbox_reg;
		}
	}

	return;

err_obj:
	i915_gem_object_put(obj);
err:
	DRM_DEBUG_DRIVER("Failed to allocate space for semaphores, disabling\n");
	i915.semaphores = 0;
}

static void intel_ring_init_irq(struct drm_i915_private *dev_priv,
				struct intel_engine_cs *engine)
{
	engine->irq_enable_mask = GT_RENDER_USER_INTERRUPT << engine->irq_shift;

	if (INTEL_GEN(dev_priv) >= 8) {
		engine->irq_enable = gen8_irq_enable;
		engine->irq_disable = gen8_irq_disable;
		engine->irq_seqno_barrier = gen6_seqno_barrier;
	} else if (INTEL_GEN(dev_priv) >= 6) {
		engine->irq_enable = gen6_irq_enable;
		engine->irq_disable = gen6_irq_disable;
		engine->irq_seqno_barrier = gen6_seqno_barrier;
	} else if (INTEL_GEN(dev_priv) >= 5) {
		engine->irq_enable = gen5_irq_enable;
		engine->irq_disable = gen5_irq_disable;
		engine->irq_seqno_barrier = gen5_seqno_barrier;
	} else if (INTEL_GEN(dev_priv) >= 3) {
		engine->irq_enable = i9xx_irq_enable;
		engine->irq_disable = i9xx_irq_disable;
	} else {
		engine->irq_enable = i8xx_irq_enable;
		engine->irq_disable = i8xx_irq_disable;
	}
}

static void i9xx_set_default_submission(struct intel_engine_cs *engine)
{
	engine->submit_request = i9xx_submit_request;
}

static void gen6_bsd_set_default_submission(struct intel_engine_cs *engine)
{
	engine->submit_request = gen6_bsd_submit_request;
}

static void intel_ring_default_vfuncs(struct drm_i915_private *dev_priv,
				      struct intel_engine_cs *engine)
{
	intel_ring_init_irq(dev_priv, engine);
	intel_ring_init_semaphores(dev_priv, engine);

	engine->init_hw = init_ring_common;
	engine->reset_hw = reset_ring_common;

	engine->context_pin = intel_ring_context_pin;
	engine->context_unpin = intel_ring_context_unpin;

	engine->request_alloc = ring_request_alloc;

	engine->emit_breadcrumb = i9xx_emit_breadcrumb;
	engine->emit_breadcrumb_sz = i9xx_emit_breadcrumb_sz;
	if (i915.semaphores) {
		int num_rings;

		engine->emit_breadcrumb = gen6_sema_emit_breadcrumb;

		num_rings = hweight32(INTEL_INFO(dev_priv)->ring_mask) - 1;
		if (INTEL_GEN(dev_priv) >= 8) {
			engine->emit_breadcrumb_sz += num_rings * 6;
		} else {
			engine->emit_breadcrumb_sz += num_rings * 3;
			if (num_rings & 1)
				engine->emit_breadcrumb_sz++;
		}
	}

	engine->set_default_submission = i9xx_set_default_submission;

	if (INTEL_GEN(dev_priv) >= 8)
		engine->emit_bb_start = gen8_emit_bb_start;
	else if (INTEL_GEN(dev_priv) >= 6)
		engine->emit_bb_start = gen6_emit_bb_start;
	else if (INTEL_GEN(dev_priv) >= 4)
		engine->emit_bb_start = i965_emit_bb_start;
	else if (IS_I830(dev_priv) || IS_I845G(dev_priv))
		engine->emit_bb_start = i830_emit_bb_start;
	else
		engine->emit_bb_start = i915_emit_bb_start;
}

int intel_init_render_ring_buffer(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;
	int ret;

	intel_ring_default_vfuncs(dev_priv, engine);

	if (HAS_L3_DPF(dev_priv))
		engine->irq_keep_mask = GT_RENDER_L3_PARITY_ERROR_INTERRUPT;

	if (INTEL_GEN(dev_priv) >= 8) {
		engine->init_context = intel_rcs_ctx_init;
		engine->emit_breadcrumb = gen8_render_emit_breadcrumb;
		engine->emit_breadcrumb_sz = gen8_render_emit_breadcrumb_sz;
		engine->emit_flush = gen8_render_ring_flush;
		if (i915.semaphores) {
			int num_rings;

			engine->semaphore.signal = gen8_rcs_signal;

			num_rings =
				hweight32(INTEL_INFO(dev_priv)->ring_mask) - 1;
			engine->emit_breadcrumb_sz += num_rings * 8;
		}
	} else if (INTEL_GEN(dev_priv) >= 6) {
		engine->init_context = intel_rcs_ctx_init;
		engine->emit_flush = gen7_render_ring_flush;
		if (IS_GEN6(dev_priv))
			engine->emit_flush = gen6_render_ring_flush;
	} else if (IS_GEN5(dev_priv)) {
		engine->emit_flush = gen4_render_ring_flush;
	} else {
		if (INTEL_GEN(dev_priv) < 4)
			engine->emit_flush = gen2_render_ring_flush;
		else
			engine->emit_flush = gen4_render_ring_flush;
		engine->irq_enable_mask = I915_USER_INTERRUPT;
	}

	if (IS_HASWELL(dev_priv))
		engine->emit_bb_start = hsw_emit_bb_start;

	engine->init_hw = init_render_ring;
	engine->cleanup = render_ring_cleanup;

	ret = intel_init_ring_buffer(engine);
	if (ret)
		return ret;

	if (INTEL_GEN(dev_priv) >= 6) {
		ret = intel_engine_create_scratch(engine, PAGE_SIZE);
		if (ret)
			return ret;
	} else if (HAS_BROKEN_CS_TLB(dev_priv)) {
		ret = intel_engine_create_scratch(engine, I830_WA_SIZE);
		if (ret)
			return ret;
	}

	return 0;
}

int intel_init_bsd_ring_buffer(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	intel_ring_default_vfuncs(dev_priv, engine);

	if (INTEL_GEN(dev_priv) >= 6) {
		/* gen6 bsd needs a special wa for tail updates */
		if (IS_GEN6(dev_priv))
			engine->set_default_submission = gen6_bsd_set_default_submission;
		engine->emit_flush = gen6_bsd_ring_flush;
		if (INTEL_GEN(dev_priv) < 8)
			engine->irq_enable_mask = GT_BSD_USER_INTERRUPT;
	} else {
		engine->mmio_base = BSD_RING_BASE;
		engine->emit_flush = bsd_ring_flush;
		if (IS_GEN5(dev_priv))
			engine->irq_enable_mask = ILK_BSD_USER_INTERRUPT;
		else
			engine->irq_enable_mask = I915_BSD_USER_INTERRUPT;
	}

	return intel_init_ring_buffer(engine);
}

int intel_init_blt_ring_buffer(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	intel_ring_default_vfuncs(dev_priv, engine);

	engine->emit_flush = gen6_ring_flush;
	if (INTEL_GEN(dev_priv) < 8)
		engine->irq_enable_mask = GT_BLT_USER_INTERRUPT;

	return intel_init_ring_buffer(engine);
}

int intel_init_vebox_ring_buffer(struct intel_engine_cs *engine)
{
	struct drm_i915_private *dev_priv = engine->i915;

	intel_ring_default_vfuncs(dev_priv, engine);

	engine->emit_flush = gen6_ring_flush;

	if (INTEL_GEN(dev_priv) < 8) {
		engine->irq_enable_mask = PM_VEBOX_USER_INTERRUPT;
		engine->irq_enable = hsw_vebox_irq_enable;
		engine->irq_disable = hsw_vebox_irq_disable;
	}

	return intel_init_ring_buffer(engine);
}
