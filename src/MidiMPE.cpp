#include "MidiMPE.hpp"


Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

	p->addModel(modelMidiMPE);

}
