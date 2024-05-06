/*
 * Copyright (c) 2024 Viasat Inc.
 * Authors:  Amber Cronin, Jae Won Chung, Mike Foxworthy, Feng Li, Mark Claypool
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "quicly/ss.h"
#include <stdint.h>
#include <math.h>

static void recalc_rho(quicly_cc_t *cc, const quicly_loss_t *loss) {
	// RTT in msec
	// Linux here uses us instead of ms for their rho calculations, which gives them more accurate information about
	// the bandwidth of the connection. Here, we only have ms level measurements from loss, and so all of our
	// measurements are only up to that level of precision. It cleans the code up a little bit though from the kernel. 
	// we have changed this to use doubles for more accuracy and ease of writing code. 
	double rho = (double) loss->rtt.minimum / QUICLY_HYBLA_RTT0;
	// don't allow the ratio to be less than one for faster connections than reference. 
	cc->ss_state.hybla.rho = MAX(rho, 1);
}

void ss_hybla(quicly_cc_t *cc, const quicly_loss_t *loss, uint32_t bytes, uint64_t largest_acked, uint32_t inflight,
                        uint64_t next_pn, int64_t now, uint32_t max_udp_payload_size)
{
	recalc_rho(cc, loss);

	cc->cwnd += (uint32_t)round(pow(2, MIN(cc->ss_state.hybla.rho, QUICLY_HYBLA_RHO_LIM)));

	if (cc->cwnd_maximum < cc->cwnd)
		cc->cwnd_maximum = cc->cwnd;
}

quicly_ss_type_t quicly_ss_type_hybla = { "hybla", ss_hybla };