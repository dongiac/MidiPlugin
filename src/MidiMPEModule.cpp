#include "MidiMPE.hpp"

/* MODULE */
struct MidiMPEModule : Module {
	enum ParamIds {
		NUM_PARAMS,
	};

	enum InputIds {
		NUM_INPUTS,
	};

	enum OutputIds {
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};


	MidiMPEModule() {

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	}


	void process(const ProcessArgs &args) override {
		;
	}


};

/* MODULE WIDGET */
struct MidiMPEModuleWidget : ModuleWidget {
	MidiMPEModuleWidget(MidiMPEModule* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HelloModule.svg")));
	}

};


Model * modelMidiMPE = createModel<MidiMPEModule, MidiMPEModuleWidget>("HelloWorld");

