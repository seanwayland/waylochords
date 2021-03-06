/*
 ==============================================================================
 
 This file was auto-generated!
 
 It contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <vector>
#include <stdlib.h>

using namespace std;


//==============================================================================
WaylochorderAudioProcessor::WaylochorderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", AudioChannelSet::stereo(), true)
#endif
                  )
#endif
{
}

WaylochorderAudioProcessor::~WaylochorderAudioProcessor()
{
}

// global variables for plugin

int playing[127]; // array to hold notes playing
int numChords; // num of chords in chord sequence
int numNotes ; // number of notes in a chord
int note; // a midi note value
int chordsPosition = 0; // pointer to postion in chord array
int midChan = 0;
int latchBool = 1;
vector<int> chord;
// initialize the chord array to 3 chords
vector< vector<int> > chords {
    { 41, 48 , 55, 56, 58, 63 },
    { 44 , 51 , 56, 58 , 61, 63 },
    { 42 , 49, 56, 60 , 61 , 65 },
    {48, 49, 56 , 58 , 63 },
    { 51, 52, 59, 61, 68 },
    {39, 46, 58, 63, 67, 68 } ,
    { 48, 58 , 63 , 65, 68 },
    {41, 48, 58 , 63, 67 } ,
    {41, 48, 57, 62, 65 },
    { 41, 48 , 55, 56, 58, 63 },
    { 44 , 51 , 56, 58 , 61, 63 },
    { 42 , 49, 56, 60 , 61 , 65 },
    {48, 49, 56 , 58 , 63 },
    { 51, 52, 59, 61, 68 },
    {39, 46, 58, 63, 67, 68 } ,
    { 48, 58 , 63 , 65, 68 },
    {41, 48, 58 , 63, 67 } ,
    {41, 48, 57, 62, 65 },
    {46 , 53 , 60, 63, 68, 72, 73 },
    { 44, 51, 58, 63, 68, 72 , 73 },
    { 43, 50 , 58 , 60 , 65 , 70,  72 , 73 },
    { 42, 49 , 56 , 60 , 65 , 70,  72 , 73 },
    { 46, 53 , 60  , 65 , 70,  72 , 73 },
    {46 , 53 , 60, 63, 68, 72, 73 },
    { 44, 51, 58, 63, 68, 72 , 73 },
    { 43, 50 , 58 , 60 , 65 , 70,  72 , 73 },
    { 42, 49 , 56 , 60 , 65 , 70,  72 , 73 },
    { 46, 53 , 60  , 65 , 70,  72 , 73 },
    { 41, 48 , 55 ,56 ,58 , 63}
};


//==============================================================================
const String WaylochorderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WaylochorderAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool WaylochorderAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool WaylochorderAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double WaylochorderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WaylochorderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int WaylochorderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WaylochorderAudioProcessor::setCurrentProgram (int index)
{
}

const String WaylochorderAudioProcessor::getProgramName (int index)
{
    return {};
}

void WaylochorderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void WaylochorderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    for (int i = 0; i < 127 ; i++ ){
        playing[i]= 0;
    }
    
}

void WaylochorderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WaylochorderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void WaylochorderAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    
    MidiMessage m;
    MidiMessage n;
    
    
    
    {
        buffer.clear();
        
        
        MidiBuffer processedMidi;
        int time;
        
        
        
        for (MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
        {
            
            
            
            if (latchBool ==0 )
            {
            
            // middle C resets the sequence
            if (m.isNoteOn() && m.getNoteNumber() == 60 ) // reset the array position and play first chord
            {
                chordsPosition = 0;
                // play the first chord in the array
                int chordSize = chords[chordsPosition].size();
                //int chordSize = 6;
                
                // for notes in current  chord
                for ( int i = 0; i < chordSize; i++) {
                    int NewNote = chords[chordsPosition][i];
                    m = MidiMessage::noteOn(m.getChannel(), NewNote, m.getVelocity());
                    processedMidi.addEvent(m, time);
                }
            }
            
            else if (m.isNoteOn() && m.getNoteNumber() > 68 ){
                
                for ( int j = 0 ; j < 127; j++)
                {
                    n = MidiMessage::noteOff(m.getChannel(), j);
                    processedMidi.addEvent(n, time);
                    
                }
                
                
            }
            
            // C# steps backwards
            // if at start do nothing
            
            else if (m.isNoteOn() && m.getNoteNumber() == 61 )
                
            {
                // move chord pointer back one space if it is not at zero
                if (chordsPosition > 0)
                { chordsPosition -= 1 ;}
                
                else
                { chordsPosition =  chords.size() - 1;}
                
                // now play chord at new pointer position
                
                int chordSize = chords[chordsPosition].size();
                
                for ( int i = 0; i < chordSize; i++) {
                    int NewNote = chords[chordsPosition][i];
                    m = MidiMessage::noteOn(m.getChannel(), NewNote, m.getVelocity());
                    processedMidi.addEvent(m, time);
                }
                
        }
            
            else if (m.isNoteOn() && m.getNoteNumber() == 62 )
                
            {
                

                
                int chordSize = chords[chordsPosition].size();
                // play chord at current position
                
                // now play chord at current pointer position
                for ( int i = 0; i < chordSize; i++) {
                    int NewNote = chords[chordsPosition][i];
                    m = MidiMessage::noteOn(m.getChannel(), NewNote, m.getVelocity());
                    processedMidi.addEvent(m, time);
                }//
                
            }
            
            else if (m.isNoteOn() && m.getNoteNumber() == 63 )
            {

                
                int chordSize = chords[chordsPosition].size();
                // play chord at current position
                
                // now play chord at current pointer position
                for ( int i = 0; i < chordSize; i++) {
                    int NewNote = chords[chordsPosition][i];
                    m = MidiMessage::noteOn(m.getChannel(), NewNote, m.getVelocity());
                    processedMidi.addEvent(m, time);
                }//
                

            }
            
            
            // D moves forwards through sequence
            // If at end go back to beginning
            else if (m.isNoteOn() && m.getNoteNumber() == 64 )
                
            {
                
                // move pointer forward unless it as end then do nothing
                if ( chordsPosition < (chords.size() -1)  )
                {
                    chordsPosition += 1;
                }
                
                else if ( chordsPosition == (chords.size() -1) )
                {
                    chordsPosition = 0;
                }
                
                else {}
                
                int chordSize = chords[chordsPosition].size();
                // play chord at current position
                
                // now play chord at current pointer position
                for ( int i = 0; i < chordSize; i++) {
                    int NewNote = chords[chordsPosition][i];
                    m = MidiMessage::noteOn(m.getChannel(), NewNote, m.getVelocity());
                    processedMidi.addEvent(m, time);
                }//
                
                
            }
            
            else if (m.isNoteOn() && m.getNoteNumber() == 65 )
                
            {
                
                // move pointer forward unless it as end then do nothing
                if ( chordsPosition < (chords.size() -1)  )
                {
                    chordsPosition += 1;
                }
                
                else if ( chordsPosition == (chords.size() -1) )
                {
                    chordsPosition = 0;
                }
                
                else {}
                
                int chordSize = chords[chordsPosition].size();
                // play chord at current position
                
                // now play chord at current pointer position
                for ( int i = 0; i < chordSize; i++) {
                    int NewNote = chords[chordsPosition][i];
                    m = MidiMessage::noteOn(m.getChannel(), NewNote, m.getVelocity());
                    processedMidi.addEvent(m, time);
                }//
            
                
            }
            
            else if (m.isNoteOn() && m.getNoteNumber() == 66 )
                
            {
                chordsPosition = rand() % (chords.size() - 1);
                
                int chordSize = chords[chordsPosition].size();
                // play chord at current position
                
                // now play chord at current pointer position
                for ( int i = 0; i < chordSize; i++) {
                    int NewNote = chords[chordsPosition][i];
                    m = MidiMessage::noteOn(m.getChannel(), NewNote, m.getVelocity());
                    processedMidi.addEvent(m, time);
                }//
                
                
            }
            else if (m.isNoteOn() && m.getNoteNumber() == 67 )
                
            {
                chordsPosition = rand() % (chords.size() - 1);
                
                int chordSize = chords[chordsPosition].size();
                // play chord at current position
                
                // now play chord at current pointer position
                for ( int i = 0; i < chordSize; i++) {
                    int NewNote = chords[chordsPosition][i];
                    m = MidiMessage::noteOn(m.getChannel(), NewNote, m.getVelocity());
                    processedMidi.addEvent(m, time);
                }//
                
                
            }
            else if (m.isNoteOn() && m.getNoteNumber() == 68 )
            {
                latchBool = 1;
            }
            else if (m.isNoteOff())
                
            {
                // send note off at current pointer position
                
                
                for (int i = 0; i < 128; ++i){
                    m = MidiMessage::noteOff(m.getChannel(), i, 0.0f);
                    processedMidi.addEvent(m, time);
                }
                
            }
            
            else if (m.isAftertouch())
            {
            }
            else if (m.isPitchWheel())
            {
            }
            
            } // end of latchbool loop
            // LATCHBOOL IS ON
            // processedMidi.addEvent (m, time);
            else {
                

                if (m.isNoteOn() && m.getNoteNumber() == 68)
                { latchBool = 0;}
                else if (m.isNoteOn() && m.getNoteNumber() != 68)
                {

                    for ( int j = 0 ; j < 127; j++)
                    { if (playing[j] == 1) {
                        n = MidiMessage::noteOff(m.getChannel(), j);
                        processedMidi.addEvent(n, time);
                        playing[j] = 0;
                    }
                    }
                    
                    
                }
                else if (m.isNoteOff() && m.getNoteNumber() == 60 ) // reset the array position and play first chord
                {
                    chordsPosition = 0;
                    // play the first chord in the array
                    int chordSize = chords[chordsPosition].size();
                    //int chordSize = 6;
                    
                    // for notes in current  chord
                    for ( int i = 0; i < chordSize; i++) {
                        int NewNote = chords[chordsPosition][i];
                        m = MidiMessage::noteOn(m.getChannel(), NewNote, 127.0f);
                        processedMidi.addEvent(m, time);
                        playing[NewNote] = 1;
                    }
                }
                
                else if (m.isNoteOff() && m.getNoteNumber() == 61 ) // reset the array position and play first chord
                {
                    
                    // move chord pointer back one space if it is not at zero
                    if (chordsPosition > 0)
                    { chordsPosition -= 1 ;}
                    
                    else
                    { chordsPosition =  chords.size() - 1;}
                    
                    int chordSize = chords[chordsPosition].size();
                    //int chordSize = 6;
                    
                    // for notes in current  chord
                    for ( int i = 0; i < chordSize; i++) {
                        int NewNote = chords[chordsPosition][i];
                        m = MidiMessage::noteOn(m.getChannel(), NewNote, 127.0f);
                        processedMidi.addEvent(m, time);
                        playing[NewNote] = 1;
                    }
                }
                
                else if (m.isNoteOff() && m.getNoteNumber() == 62 ) // reset the array position and play first chord
                {

                    
                    int chordSize = chords[chordsPosition].size();
                    //int chordSize = 6;
                    
                    // for notes in current  chord
                    for ( int i = 0; i < chordSize; i++) {
                        int NewNote = chords[chordsPosition][i];
                        m = MidiMessage::noteOn(m.getChannel(), NewNote, 127.0f);
                        processedMidi.addEvent(m, time);
                        playing[NewNote] = 1;
                    }
                }
                
                else if (m.isNoteOff() && m.getNoteNumber() == 63 ) // reset the array position and play first chord
                {
                    
                    
                    int chordSize = chords[chordsPosition].size();
                    //int chordSize = 6;
                    
                    // for notes in current  chord
                    for ( int i = 0; i < chordSize; i++) {
                        int NewNote = chords[chordsPosition][i];
                        m = MidiMessage::noteOn(m.getChannel(), NewNote, 127.0f);
                        processedMidi.addEvent(m, time);
                        playing[NewNote] = 1;
                    }
                }
                
                else if (m.isNoteOff() && m.getNoteNumber() == 64 ) // reset the array position and play first chord
                {
                    // move pointer forward unless it as end then do nothing
                    if ( chordsPosition < (chords.size() -1)  )
                    {
                        chordsPosition += 1;
                    }
                    
                    else if ( chordsPosition == (chords.size() -1) )
                    {
                        chordsPosition = 0;
                    }
                    
                    else {}
                    
                    int chordSize = chords[chordsPosition].size();
                    //int chordSize = 6;
                    
                    // for notes in current  chord
                    for ( int i = 0; i < chordSize; i++) {
                        int NewNote = chords[chordsPosition][i];
                        m = MidiMessage::noteOn(m.getChannel(), NewNote, 127.0f);
                        processedMidi.addEvent(m, time);
                        playing[NewNote] = 1;
                    }
                }
                
                else if (m.isNoteOff() && m.getNoteNumber() == 65 ) // reset the array position and play first chord
                {
                    // move pointer forward unless it as end then do nothing
                    if ( chordsPosition < (chords.size() -1)  )
                    {
                        chordsPosition += 1;
                    }
                    
                    else if ( chordsPosition == (chords.size() -1) )
                    {
                        chordsPosition = 0;
                    }
                    
                    else {}
                    
                    int chordSize = chords[chordsPosition].size();
                    //int chordSize = 6;
                    
                    // for notes in current  chord
                    for ( int i = 0; i < chordSize; i++) {
                        int NewNote = chords[chordsPosition][i];
                        m = MidiMessage::noteOn(m.getChannel(), NewNote, 127.0f);
                        processedMidi.addEvent(m, time);
                        playing[NewNote] = 1;
                    }
                }
                
                else if (m.isNoteOff() && m.getNoteNumber() == 66 ) // reset the array position and play first chord
                {
                    // move pointer forward unless it as end then do nothing

                    chordsPosition = rand() % (chords.size() - 1);
                    int chordSize = chords[chordsPosition].size();
                    //int chordSize = 6;
                    
                    // for notes in current  chord
                    for ( int i = 0; i < chordSize; i++) {
                        int NewNote = chords[chordsPosition][i];
                        m = MidiMessage::noteOn(m.getChannel(), NewNote, 127.0f);
                        processedMidi.addEvent(m, time);
                        playing[NewNote] = 1;
                    }
                }
                
                else if (m.isNoteOff() && m.getNoteNumber() == 67 ) // reset the array position and play first chord
                {
                    // move pointer forward unless it as end then do nothing
                    
                    chordsPosition = rand() % (chords.size() - 1);
                    int chordSize = chords[chordsPosition].size();
                    //int chordSize = 6;
                    
                    // for notes in current  chord
                    for ( int i = 0; i < chordSize; i++) {
                        int NewNote = chords[chordsPosition][i];
                        m = MidiMessage::noteOn(m.getChannel(), NewNote, 127.0f);
                        processedMidi.addEvent(m, time);
                        playing[NewNote] = 1;
                    }
                }
                
                else if (m.isAftertouch())
                {
                }
                else if (m.isPitchWheel())
                {
                }
                
                
                
                

                
                
                
            }
        }
        
        midiMessages.swapWith (processedMidi);
    }

    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        // ..do something to the data...
    }
}



//==============================================================================
bool WaylochorderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* WaylochorderAudioProcessor::createEditor()
{
    return new WaylochorderAudioProcessorEditor (*this);
}

//==============================================================================
void WaylochorderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void WaylochorderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WaylochorderAudioProcessor();
}

