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

	uint8_t notes[16];//vettore dove vengono salvate le note 
	uint8_t strike[16]; //note on velocity
	uint8_t lift[16]; //note off velocity
	uint8_t press[16]; //aftertouch
	uint16_t glide[16]; //va da 0 a 16535 con 8192 posizione neutrale
	uint8_t slide[16]; //0xb il controller[data 0] è il 74, il valore[data 1] 0-127
	
	//variabile bool [On / OFF] per mandare segnale di gate, brutto ma per ora è così
	bool gates[16] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};  

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

		//settare i Voltage di tutti i channel
		for(int channel = 0; channel<16; channel++){
			//ci sono da 0 a 11 ottave, ogni ottava è 12 "unità", se pongo 0V = C5 (60) allora vado da -5V(C0) a 5V(C11)
			//qualsiasi è la nota sottraggo 60, centrandomi in C5 e divido per le 12 unità ottenendo 1V per ottava
			outputs[VOCT].setVoltage((notes[channel] - 60.f) - 12.f, channel);
			//controlla il gate di ogni canale, se true 10v se false 0V
			outputs[GATE].setVoltage(gates[channel] ? 10.f : 0.f, channel);

		}



		while (midiInput.shift(&msg)) {
			
			switch(msg.getStatus()){

				case 0x8: {
					//Note OFF
					int channel = msg.getChannel(); 
					notes[channel] = msg.getNote();
					gates[channel] = false;	
						//Prendo qui la Velocity (Lift)
						lift[channel] = msg.getValue();
				} break;

				case 0x9: {
					//Note ON
					int channel = msg.getChannel(); 
					notes[channel] = msg.getNote();
					gates[channel] = true;					
						// prendo qui la Velocity (Strike)
						strike[channel] = msg.getValue();
				} break;

				case 0xa: {
					//Aftertouch (Press)
				} break;

				case 0xe: {
					//Pitch Wheel (Glide)
				} break;

				case 0xb: {
					//CC74 (Slide)
				} break;
				default: break;



			}

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