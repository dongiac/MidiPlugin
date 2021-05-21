#include "MidiMPE.hpp"

/* MODULE */
struct MidiMPEModule : Module {
	enum ParamIds {
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT1,
		NUM_INPUTS,
	};

	enum OutputIds {
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

	midi::InputQueue midiInput;


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
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MidiPlugin.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//Modulo per scelta ingressi MIDI 
		MidiWidget* midiWidget = createWidget<MidiWidget>(mm2px(Vec(3.41891, 14.8373)));
		midiWidget->box.size = mm2px(Vec(33.840, 28));
		midiWidget->setMidiPort(module ? &module->midiInput : NULL);
		addChild(midiWidget);
	}

	
/*	void appendContextMenu(Menu* menu) override {
		MidiMPEModule* module = dynamic_cast<MidiMPEModule*>(this->module);

		menu->addChild(new MenuEntry);

	
	}
*/
};


Model * modelMidiMPE = createModel<MidiMPEModule, MidiMPEModuleWidget>("MidiMPE");

