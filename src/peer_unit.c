/*
 * Copyright contributors to Specasm
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "peer.h"
#include "state.h"
#include <string.h>

struct peer_save_t_ {
	uint16_t checksum;
	uint8_t state[sizeof(state)];
};

typedef struct peer_save_t_ peer_save_t;

/*
 * This is big and ideally we'd put it in its own bank.  But the
 * version of z88dk we're using doesn't seem capable of putting
 * data in banks.  So we're just going to have to hack it.
 */

#if !defined(SPECASM_NEXT_BANKED) && !defined(SPECASM_128_BANKED)
static peer_save_t save_state;
#endif

#if defined(SPECASM_NEXT_BANKED) || defined(SPECASM_128_BANKED)
void specasm_peer_write_state_banked_e(const char *fname, uint16_t checksum)
{
	peer_save_t *save_state = (peer_save_t *) 0xc000;

	save_state->checksum = checksum;
	memcpy(&save_state->state, &state, sizeof(state));
}

#else
void specasm_peer_write_state_e(const char *fname, uint16_t checksum)
{
	save_state.checksum = checksum;
	memcpy(&save_state.state, &state, sizeof(state));
}
#endif

#if defined(SPECASM_NEXT_BANKED) || defined(SPECASM_128_BANKED)
uint16_t specasm_peer_read_state_banked_e(const char *fname)
{
	peer_save_t *save_state = (peer_save_t *) 0xc000;

	memcpy(&state, &save_state->state, sizeof(state));
	return save_state->checksum;
}
#else
uint16_t specasm_peer_read_state_e(const char *fname)
{
	memcpy(&state, &save_state.state, sizeof(state));
	return save_state.checksum;
}
#endif
