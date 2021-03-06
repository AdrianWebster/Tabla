//
//  Instrument.hpp
//  PaperBounce3
//
//  Created by Luke Iannini on 11/14/16.
//
//

#ifndef Instrument_hpp
#define Instrument_hpp

#include <stdio.h>
#include "GameWorld.h"
#include "FileWatch.h"
#include "PureDataNode.h"
#include "RtMidi.h"
#include "Cinder-Serial.h"
#include "OpenSFZNode.h"


using namespace cipd;
using namespace Cinder::Serial;

class Score;

typedef int NoteNum;

typedef vector<NoteNum> Scale;


// Arpeggiator
enum class ArpMode
{
	None,
	Up,
	Down,
	UpDown,
	Random
};

struct Arpeggiator
{
	ArpMode Mode=ArpMode::None;
	int Step=0;
	NoteNum RootNote=0;
	float LastNoteTime=0;
	float NoteDuration=0;
};


// meta-params
enum class MetaParam
{
	Scale=0,
	RootNote, // Global transposition
	Tempo,
	//		BeatCount, // Score Resolution in X (i.e. number of beat divisions)
	//      NoteCount  // Score Resolution in Y (i.e. number of notes accessible)
	kNumMetaParams
};

class MetaParamInfo
{
public:
	bool  isDiscrete() const { return mNumDiscreteStates!=-1; }
	float discretize( float ) const; // still a fractional value, but rounded off to # of discrete states
	
	int mNumDiscreteStates=-1;
	float mDefaultValue=0.f; // 0..1
};


// MIDI
struct tOnNoteInfo
{
	float mStartTime;
	float mDuration;
};
// (note) -> (start time, duration)

typedef std::shared_ptr<RtMidiOut> RtMidiOutRef;



// instrument info
class Instrument
{
public:

	~Instrument();

	void setParams( XmlTree );
	void setup();

	bool isNoteType() const;
	bool isAvailable() const; // e.g. couldn't connect to device
	
	// colors!
	ColorA mPlayheadColor;
	ColorA mScoreColor, mScoreColorDownLines;
	ColorA mNoteOffColor, mNoteOnColor;

	gl::TextureRef mIcon;
	string mIconFileName;
	vec2 mIconGradientCenter=vec2(.5,.5);
	
	//
	string mName;

	//
	enum class SynthType
	{
		Additive    = 1,
		MIDI	    = 2,
		RobitPokie  = 3,
		Meta	    = 4, // controls global params
		Sampler     = 5
	};

	SynthType mSynthType;
	MetaParam mMetaParam; // only matters if mSynthType==Meta
	MetaParamInfo mMetaParamInfo;

public:
	typedef vector<Score const*> Scores;
	void updateSynthesis( const Scores& );
	void updateSynthesisWithVision( const Scores& );
	void killAllNotes();

	void setPokieRobitPulseTime( float t ) { mPokieRobitPulseTime=t; }

	void setAdditiveSynthID( int id ) { mAdditiveSynthID=id; }
	void setPd( PureDataNodeRef pd ) { mPd=pd; }
	
private:
	int mAdditiveSynthID;
	PureDataNodeRef mPd;
	
	OpenSFZNodeRef mOpenSFZNode;
	
	// serial (arduino/robits)
	SerialDeviceRef mSerialDevice;

	void setupSerial();
	void sendSerialByte(const uint8_t charByte, const uint8_t hiLowByte);

	float mPokieRobitPulseTime=0.01;

	uint8_t serialCharForNote(int note);

	// midi
	RtMidiOutRef mMidiOut;

	int  mPort=0;
	int  mChannel=0;

	void setupMIDI();

	int  mMapNotesToChannels=0; // for KORG Volca Sample (0 == off, N>0 == map notes across N channels)
	int  channelForNote( int note ) const;

	// update synthesis -- by synth type
	void updateNoteSynthesis( const Scores& );
	void updateAdditiveSynthesis( const Scores& ) const;
	void updateAdditiveSynthesisWithVision( const Scores& ) const;	

	// note managment
	map<NoteNum, tOnNoteInfo> mOnNotes;
	Arpeggiator mArpeggiator;

	void doNoteOn( int note, float duration ); // start time is now
	void doNoteOff( int note );

	int  noteForY( const Score&, int y ) const;
	bool isNoteInFlight( int note ) const;
	void updateNoteOffs();

	void tickArpeggiator();
	
	// midi convenience methods
	void sendMidi( RtMidiOutRef, uchar, uchar, uchar );
	void sendNoteOn ( RtMidiOutRef midiOut, uchar channel, uchar note, uchar velocity );
	void sendNoteOff ( RtMidiOutRef midiOut, uchar channel, uchar note );

};

typedef std::shared_ptr<Instrument> InstrumentRef;

class InstrumentRefs : public vector<InstrumentRef>
{
public:
	InstrumentRef hasSynthType ( Instrument::SynthType ) const; // returns 1st if present
	bool		  hasInstrument( InstrumentRef ) const;
	InstrumentRef hasNoteType  () const; // returns 1st if present
};

#endif /* Instrument_hpp */
