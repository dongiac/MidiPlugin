#include "MidiMPE.hpp"

using namespace std;

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
		VOCT,
		GATE,
		STRIKE,
		PRESS,
		GLIDE,
		SLIDE,
		LIFT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NOTEONLIGHT,
		NUM_LIGHTS,
	};

	midi::InputQueue midiInput; 


	MidiMPEModule() {

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	}


	void process(const ProcessArgs &args) override {

		midi::Message msg;
		
		int NumberOfChannels = 15;
	

		//Setto gli Output tutti a NumberOfChannels
		outputs[VOCT].setChannels(NumberOfChannels);
		outputs[GATE].setChannels(NumberOfChannels);
		outputs[STRIKE].setChannels(NumberOfChannels);
		outputs[PRESS].setChannels(NumberOfChannels);
		outputs[GLIDE].setChannels(NumberOfChannels);
		outputs[SLIDE].setChannels(NumberOfChannels);
		outputs[LIFT].setChannels(NumberOfChannels);


		while (midiInput.shift(&msg)) {


		}
	}


};



/* MODULE WIDGET */
struct MidiMPEModuleWidget : ModuleWidget {
	MidiMPEModuleWidget(MidiMPEModule* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MidiPlugin.svg")));

		//inserisce le quattro viti nella GUI
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//Modulo per scelta ingressi MIDI 
		MidiWidget* midiWidget = createWidget<MidiWidget>(mm2px(Vec(5, 20)));
		midiWidget->box.size = mm2px(Vec(47, 28));
		midiWidget->setMidiPort(module ? &module->midiInput : NULL);
		addChild(midiWidget);

		//Mette sulla GUI del module il led indicato nell'ultimo parametro 
		//addChild(createLightCentered<LargeLight<GreenLight>>(Vec(50, NumberOfChannels5), module, MidiMPEModule::NOTEONLIGHT));

		//Output generici MIDI
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(17,70.4)), module, MidiMPEModule::VOCT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(17,98.7)), module, MidiMPEModule::GATE));

		//Output Specifici MPE
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,56)), module, MidiMPEModule::STRIKE));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,70.4)), module, MidiMPEModule::PRESS));		
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,84.5)), module, MidiMPEModule::GLIDE));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,98.7)), module, MidiMPEModule::SLIDE));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,113)), module, MidiMPEModule::LIFT));

	}

};

Model * modelMidiMPE = createModel<MidiMPEModule, MidiMPEModuleWidget>("MidiMPE");