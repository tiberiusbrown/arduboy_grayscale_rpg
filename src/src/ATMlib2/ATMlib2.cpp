
#include "ATMlib2.h"


void ATMsynth::setup(void) {
	if (!setup_done) {
		atm_synth_setup();
		setup_done = true;
	}
}

void ATMsynth::play(const uint8_t *score) {
	setup();
	atm_synth_start_score(score);
}

// Stop playing, unload melody
void ATMsynth::stop() {
	atm_synth_set_score_paused(1);
}

// Start grinding samples or Pause playback
void ATMsynth::playPause() {
	atm_synth_set_score_paused(atm_synth_is_score_playing());
}
