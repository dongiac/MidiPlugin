#include "MidiMPE.hpp"

using namespace std;

/* MODULE */
struct MidiMPEModule : Module {
	enum ParamIds {
		MODE_POLY,
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
		MODWHEEL,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NOTEONLIGHT,
		NUM_LIGHTS,
	};

	bool modePoly = params[MODE_POLY].getValue(); //1 MPE 0 Rotative

	uint8_t notes[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//vettore dove vengono salvate le note 
	uint8_t strike[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //note on velocity
	uint8_t lift[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //note off velocity
	uint8_t press[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //aftertouch
	uint8_t slide[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //0xb il controller[data 0] è il 74, il valore[data 1] 0-127
	uint8_t modwheel[16] {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//variabile bool [On / OFF] per mandare segnale di gate
	bool gates[16] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};  
	// Inizializza i Glide in posizione Neutra
	float glide[16] = {8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,8192.f,};

	midi::InputQueue midiInput; 

	MidiMPEModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(MODE_POLY, 0.f, 1.f, 1.f,"Rotative MPE");
	}

	void process(const ProcessArgs &args) override {


		midi::Message msg;
		
		int numberOfChannels = 15;

		while (midiInput.shift(&msg)) {

			switch(msg.getStatus()){

				case 0x8: {
					//Note OFF
					noteOFF(msg.getNote());
				} break;

				case 0x9: {
					//Note ON
					if(msg.getValue() > 0){
						int channel = msg.getChannel();
						noteON(msg.getNote(), &channel);
						strike[channel] = msg.getValue(); //Velocity
					}else noteOFF(msg.getNote()); //Note OFF

				} break;

				case 0xa: {
					//Poly Pressure
					//Aftertouch (Press)
					if (modePoly){
						press[msg.getChannel()] = msg.getValue();
						
					}
					else for(int channel = 0; channel<16; channel++){
						if (notes[channel] == msg.getNote()){
							press[channel] = msg.getValue();

						}
					}

				} break;

				case 0xd: {
					//Channel Pressure
					//Aftertouch (Press)
					if (modePoly){
						press[msg.getChannel()] = msg.getValue();
						
					}
					else {
						for(int channel = 0; channel<16; channel++){	
							press[channel] = msg.getValue();			
						}
					}
									
				} break;

				case 0xe: {
					//Pitch Wheel (Glide)
					//Ho LSB (00-7F) su Note e MSB (00-7F) su Value, shifto value e sommo note
					if(modePoly){
						glide[msg.getChannel()] = ((uint16_t) msg.getValue() << 7) | msg.getNote();
					} else glide[0] = ((uint16_t) msg.getValue() << 7) | msg.getNote();
						
				} break;

				case 0xb: {
					//CC74 (Slide)
					if(msg.getNote() == 74){ //la roli manda sul controller 74
						slide[msg.getChannel()] = msg.getValue();
					}
					//CC01 Mod Controller
					if(msg.getNote() == 01){
						if(modePoly){
							modwheel[msg.getChannel()] = msg.getValue();	
						}
						else modwheel[0] = msg.getValue();
						
					}
					/*
					//CC40 Midi End
					if(msg.getNote() == 40){
						if (msg.getValue() == 0){
							gates[msg.getChannel()] = false;
						}	
					}*/
				} break;

				default: break;
			}
		}
		
		//Set degli Output tutti a numberOfChannels
		outputs[VOCT].setChannels(numberOfChannels);
		outputs[GATE].setChannels(numberOfChannels);
		outputs[STRIKE].setChannels(numberOfChannels);
		outputs[PRESS].setChannels(numberOfChannels);
		outputs[SLIDE].setChannels(numberOfChannels);
		outputs[LIFT].setChannels(numberOfChannels);

		if(modePoly){
			outputs[GLIDE].setChannels(numberOfChannels);
			outputs[MODWHEEL].setChannels(numberOfChannels);
			for(int channel = 0; channel<16; channel++){
				outputs[MODWHEEL].setVoltage((modwheel[channel] / 127.f) * 10.f, channel);
				// output glide tra -5V e 5V
				outputs[GLIDE].setVoltage(((((glide[channel]) * 10.f )/ 16384.f ) - 5.f), channel);
			}
		}else{
			outputs[GLIDE].setChannels(1);
			outputs[MODWHEEL].setChannels(1);
			outputs[MODWHEEL].setVoltage((modwheel[0] / 127.f) * 10.f);
			// output glide tra -5V e 5V
			outputs[GLIDE].setVoltage(((((glide[0]) * 10.f )/ 16384.f ) - 5.f));
		}
		//settare i Voltage di tutti i channel
		for(int channel = 0; channel<16; channel++){

			//ci sono da 0 a 11 ottave, ogni ottava è 12 "unità", se pongo 0V = C5 (60) allora vado da -5V(C0) a 5V(C11)
			//qualsiasi è la nota sottraggo 60, centrandomi in C5 e divido per le 12 unità ottenendo 1V per ottava
			outputs[VOCT].setVoltage((notes[channel] - 60.f) / 12.f, channel);

			//controlla il gate di ogni canale, se true 10v se false 0V
			outputs[GATE].setVoltage(gates[channel] ? 10.f : 0.f, channel);

			//output da range 0-127 a 0-10
			outputs[STRIKE].setVoltage((strike[channel] / 127.f) * 10.f, channel);
			outputs[LIFT].setVoltage((lift[channel] / 127.f) * 10.f, channel);
			outputs[PRESS].setVoltage((press[channel] / 127.f) * 10.f, channel);
			outputs[SLIDE].setVoltage((slide[channel] / 127.f) * 10.f, channel);
			
		
		}

		
	}

	int assignChannel(uint8_t note){

		int channel= 0;
		for (channel = 0; channel < 16; channel++){
			if(!gates[channel]){
				return channel;
			}
		}
		if(channel >=16) return 0;
		else return channel;

	}

	void noteON(uint8_t note, int* channel){

		if(!modePoly){
			*channel = assignChannel(note);
		}
		notes[*channel] = note;
		gates[*channel] = true;
	}

	void noteOFF(uint8_t note){

		for (int channel = 0; channel < 16; channel++) {
			if (notes[channel] == note) {
				gates[channel] = false;
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
		
		//Crea un interruttore per scegliere MPE o Rotative
		addParam(createParam<CKSS>(mm2px(Vec(22, 55)), module, MidiMPEModule::MODE_POLY));

		//Output generici MIDI
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(17,70.4)), module, MidiMPEModule::VOCT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(17,98.7)), module, MidiMPEModule::GATE));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(17,84)), module, MidiMPEModule::MODWHEEL));

		//Output Specifici MPE
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,56)), module, MidiMPEModule::STRIKE));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,70.4)), module, MidiMPEModule::PRESS));		
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,84.5)), module, MidiMPEModule::GLIDE));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,98.7)), module, MidiMPEModule::SLIDE));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(42,113)), module, MidiMPEModule::LIFT));

	}
};
Model * modelMidiMPE = createModel<MidiMPEModule, MidiMPEModuleWidget>("MidiMPE");