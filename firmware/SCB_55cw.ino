#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
//#define SSD1306_NO_SPLASH - NG Change in Adafruit_SSD1306.h
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

// =========================================================================
// 1. HARDWARE PIN CONFIGURATION
// =========================================================================
#define ENC_SW_PIN       0   // PB0: Encoder Push Button
#define ENC_A_PIN        1   // PB1: Encoder Phase A
#define ENC_B_PIN       24   // PE6: Encoder Phase B - Changed to INT6
#define SCB55_RESET_PIN  2   // PB2: Connected to SCB-55 !Reset Line
#define RESET_BUTTON_PIN 10  // PC7: Pin connected to external reset button
#define OLED_SCL         5   // PD0: I2C SCL
#define OLED_SDA         6   // PD1: I2C SDA
#define POT_PIN         12   // A10:  Volume Potentiometer Wiper
#define SERIAL1_TX_BUFFER_SIZE 256 // (change in teensy/HardwareSerial.cpp)
#define RX_BUFFER_SIZE 128 // (change in teensy/HardwareSerial.cpp) was 64
//#define USB_MIDI_SYSEX_MAX 79 change in ../usb_midi/usb_api.h should be >75 (for screen pixel data)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 400000UL, 400000UL);

Encoder myEnc(ENC_A_PIN, ENC_B_PIN);

// =========================================================================
// 2. DATA STRUCTURES & PROGMEM LOOKUP TABLES
// =========================================================================
struct InstrumentAllocation {
    uint8_t bank;
    uint8_t program;
    char name[16]; 
};
// Melodic Instruments Table (354 sounds for Chans 1-9, 11-16)
const InstrumentAllocation scb55_bank[354] PROGMEM = {
    // SECTION 1: GENERAL MIDI CAPITAL TONES (BANK 0) - [128 Tones] Fixed
    {0, 0, "Piano 1"},        {0, 1, "Piano 2"},        {0, 2, "Piano 3"},        {0, 3, "Honky-tonk"},
    {0, 4, "E.Piano 1"},      {0, 5, "E.Piano 2"},      {0, 6, "Harpsichord"},    {0, 7, "Clav."},
    {0, 8, "Celesta"},        {0, 9, "Glockenspiel"},   {0, 10, "Music Box"},     {0, 11, "Vibraphone"},
    {0, 12, "Marimba"},       {0, 13, "Xylophone"},     {0, 14, "Tubular-bell"},  {0, 15, "Santur"},
    {0, 16, "Organ 1"},       {0, 17, "Organ 2"},       {0, 18, "Organ 3"},       {0, 19, "Church Organ"},
    {0, 20, "Reed Organ"},    {0, 21, "Accordion Fr."}, {0, 22, "Harmonica"},     {0, 23, "Bandoneon"},
    {0, 24, "Nylon-str.Gt"},  {0, 25, "Steel-str.Gt"},  {0, 26, "Jazz Guitar"},   {0, 27, "Clean Guitar"},
    {0, 28, "Muted Guitar"},  {0, 29, "Overdrive Gt"},  {0, 30, "Distortion Gt"}, {0, 31, "Gt.Harmonics"},
    {0, 32, "Acoustic Bass"}, {0, 33, "Fingered Bass"}, {0, 34, "Picked Bass"},   {0, 35, "Fretless Bass"},
    {0, 36, "Slap Bass 1"},   {0, 37, "Slap Bass 2"},   {0, 38, "Synth Bass 1"},  {0, 39, "Synth Bass 2"},
    {0, 40, "Violin"},        {0, 41, "Viola"},         {0, 42, "Cello"},         {0, 43, "Contrabass"},
    {0, 44, "Tremolo Str"},   {0, 45, "Pizzicato Str"}, {0, 46, "OrchestralHarp"},{0, 47, "Timpani"},
    {0, 48, "String Ens. 1"}, {0, 49, "String Ens. 2"}, {0, 50, "SynthStrings 1"},{0, 51, "SynthStrings 2"},
    {0, 52, "Choir Aahs"},    {0, 53, "Voice Oohs"},    {0, 54, "Synth Vox"},     {0, 55, "Orchestra Hit"},
    {0, 56, "Trumpet"},       {0, 57, "Trombone"},      {0, 58, "Tuba"},          {0, 59, "Muted Trumpet"},
    {0, 60, "French Horn"},   {0, 61, "Brass Section1"},{0, 62, "Synth Brass 1"}, //Fixed shift
    {0, 63, "Synth Brass 2"}, {0, 64, "Soprano Sax"},   {0, 65, "Alto Sax"},      {0, 66, "Tenor Sax"},
    {0, 67, "Baritone Sax"},  {0, 68, "Oboe"},          {0, 69, "English Horn"},  {0, 70, "Bassoon"},
    {0, 71, "Clarinet"},      {0, 72, "Piccolo"},       {0, 73, "Flute"},         {0, 74, "Recorder"},
    {0, 75, "Pan Flute"},     {0, 76, "Bottle Blow"},   {0, 77, "Shakuhachi"},    {0, 78, "Whistle"},
    {0, 79, "Ocarina"},       {0, 80, "Square Wave"},   {0, 81, "Saw Wave"},      {0, 82, "Syn Calliope"},
    {0, 83, "Chiffer Lead"},  {0, 84, "Charang"},       {0, 85, "Solo Vox"},      {0, 86, "5th Saw Wave"},
    {0, 87, "Bass & Lead"},   {0, 88, "Fantasia"},      {0, 89, "Warm Pad"},      {0, 90, "Polysynth"},
    {0, 91, "Space Voice"},   {0, 92, "Bowed Glass"},   {0, 93, "Metal Pad"},     {0, 94, "Halo Pad"},
    {0, 95, "Sweep Pad"},     {0, 96, "Ice Rain"},      {0, 97, "Soundtrack"},    {0, 98, "Crystal"},
    {0, 99, "Atmosphere"},    {0, 100, "Brightness"},   {0, 101, "Goblin"},       {0, 102, "Echo Drops"},
    {0, 103, "Star Theme"},   {0, 104, "Sitar"},        {0, 105, "Banjo"},        {0, 106, "Shamisen"},
    {0, 107, "Koto"},         {0, 108, "Kalimba"},      {0, 109, "Bagpipe"},      {0, 110, "Fiddle"},
    {0, 111, "Shanai"},       {0, 112, "Tinkle Bell"},  {0, 113, "Agogo"},        {0, 114, "Steel Drums"},
    {0, 115, "Woodblock"},    {0, 116, "Taiko Drum"},   {0, 117, "Melodic Tom"},  {0, 118, "Synth Drum"},
    {0, 119, "ReverseCymbal"},{0, 120, "Gt.Fret Noise"},{0, 121, "Breath Noise"}, {0, 122, "Seashore"},
    {0, 123, "Bird Tweet"},   {0, 124, "Telephone 1"},  {0, 125, "Helicopter"},   {0, 126, "Applause"},
    {0, 127, "Gunshot"},

    // SECTION 2: ROLAND GS VARIATION TONES (BANKS 1 - 64) - [98 Tones] Fixed & Sorted (98)
    {1, 38, "Synth Bass 101"}, {1, 57, "Trombone 2"},     {1, 60, "French Horn 2"},  {1, 80, "Square"}, 
    {1, 81, "Saw"},            {1, 98, "Syn Mallet"},     {1, 102, "Echo Bell"},     {1, 104, "Sitar 2"}, 
    {1, 120, "Gt. Cut Noise"}, {1, 121, "Fl.Key Click"},  {1, 122, "Rain"},          {1, 123, "Dog Bark"}, 
    {1, 124, "Telephone 2"},   {1, 125, "Car Engine"},    {1, 126, "Laughing"},      {1, 127, "Machine Gun"}, 
    {2, 102, "Echo Pan"},      {2, 120, "String Slap"},   {2, 122, "Thunder"},       {2, 123, "Horse Gallop"}, 
    {2, 124, "Door Creaking"}, {2, 125, "Car Stop"},      {2, 126, "Screaming"},     {2, 127, "Lasergun"}, 
    {3, 122, "Wind"},          {3, 123, "Bird Tweet 2"},  {3, 124, "Door Shut"},     {3, 125, "Car Pass"}, 
    {3, 126, "Punch"},         {3, 127, "Explosion"},     {4, 122, "Stream"},        {4, 124, "Scratch Mix"}, 
    {4, 125, "Car Crash"},     {4, 126, "Heartbeat"},     {5, 122, "Bubble"},        {5, 124, "Wind Chime"}, 
    {5, 125, "Siren"},         {5, 126, "Footsteps"},     {6, 125, "Train"},         {7, 125, "Jetplane"}, 
    {8, 0, "Piano 1w"},        {8, 1, "Piano 2w"},        {8, 2, "Piano 3w"},        {8, 3, "Honky-tonk w"}, 
    {8, 4, "Detuned EP 1"},    {8, 5, "Detuned EP 2"},    {8, 6, "Coupled Hps."},    //{8, 7, "Clav. w"},
    {8, 11, "Vibraphone w"},   {8, 12, "Marimba w"},      {8, 14, "Church Bell"},    {8, 16, "Detuned Organ 1"}, 
    {8, 17, "Detuned Organ 2"},{8, 19, "Church Organ 2"}, {8, 21, "Accordion It."},  {8, 24, "Ukulele"}, 
    {8, 25, "12-str.Guitar"},  {8, 26, "Hawaiian Gt."},   {8, 27, "Chorus Gt."},     {8, 28, "Funk Gt."}, 
    {8, 30, "Feedback Gt."},   {8, 31, "Gt. Feedback"},   {8, 38, "Synth Bass 3"},   {8, 39, "Synth Bass 4"}, 
    {8, 40, "Slow Violin"},    {8, 48, "Orchestra"},      {8, 50, "Synth Strings3"}, {8, 61, "Brass 2"}, 
    {8, 62, "Synth Brass 3"},  {8, 63, "Synth Brass 4"},  {8, 80, "Sine Wave"},      {8,81, "Doctor Solo"}, {8, 107, "Taisho Koto"}, 
    {8, 115, "Castanets"},     {8, 116, "Concert BassDr"},{8, 117, "Melo. Tom 2"},   {8, 118, "808 Tom"}, 
    {8, 125, "Starship"},      {9, 14, "Carillon"},       {9, 118, "Elec Perc"},     {9, 125, "Burst Noise"}, 
    {16, 0, "Piano 1d"},       {16, 4, "E.Piano 1v"},     {16, 5, "E.Piano 2v"},     {16, 6, "Harpsichord w"}, 
    {16, 16, "60s Organ 1"},   {16, 19, "Church Organ 3"},{16, 24, "Nylon Gt. o"},   {16, 25, "Mandolin"}, 
    {16, 28, "Funk Gt. 2"},    {16, 39, "Rubber Bass"},   {16, 62, "Analog Brass 1"},{16, 63, "Analog Brass 2"}, 
    {24, 4, "60s E.Piano"},    {24, 6, "Harpsi.o"},       {32, 16, "Organ 4"},       {32, 17, "Organ 5"}, 
    {32, 24, "Nylon Gt. 2"},   {32, 52, "Choir Aahs 2"}, 

    // SECTION 3: VINTAGE MT-32 LEGACY EMULATION (BANK 127) - [128 Tones] Fixed
    {127, 0, "MT32 AcouPiano1"},  {127, 1, "MT32 AcouPiano2"},   {127, 2, "MT32 AcouPiano3"},  {127, 3, "MT32 ElecPiano1"},
    {127, 4, "MT32 ElecPiano2"},  {127, 5, "MT32 ElecPiano3"},   {127, 6, "MT32 ElecPiano4"},  {127, 7, "MT32 Honkytonk"},
    {127, 8, "MT32 Elec Org 1"},  {127, 9, "MT32 Elec Org 2"},   {127, 10, "MT32 Elec Org 3"}, {127, 11, "MT32 Elec Org 4"},
    {127, 12, "MT32 Pipe Org 1"}, {127, 13, "MT32 Pipe Org 2"},  {127, 14, "MT32 Pipe Org 3"}, {127, 15, "MT32 Accordion"},
    {127, 16, "MT32 Harpsi 1"},   {127, 17, "MT32 Harpsi 2"},    {127, 18, "MT32 Harpsi 3"},   {127, 19, "MT32 Clavi 1"},
    {127, 20, "MT32 Clavi 2"},    {127, 21, "MT32 Clavi 3"},     {127, 22, "MT32 Celesta 1"},  {127, 23, "MT32 Celesta 2"},
    {127, 24, "MT32 Syn Brass1"}, {127, 25, "MT32 Syn Brass2"},  {127, 26, "MT32 Syn Brass3"}, {127, 27, "MT32 Syn Brass4"},
    {127, 28, "MT32 Syn Bass 1"}, {127, 29, "MT32 Syn Bass 2"},  {127, 30, "MT32 Syn Bass 3"}, {127, 31, "MT32 Syn Bass 4"},
    {127, 32, "MT32 Fantasy"},    {127, 33, "MT32 Harmo Pan"},   {127, 34, "MT32 Chorale"},    {127, 35, "MT32 Glasses"},
    {127, 36, "MT32 Soundtrack"}, {127, 37, "MT32 Atmosphere"},  {127, 38, "MT32 Warm Bell"},  {127, 39, "MT32 Funny Vox"},
    {127, 40, "MT32 Echo Bell"},  {127, 41, "MT32 Ice Rain"},    {127, 42, "MT32 Oboe 2001"},  {127, 43, "MT32 Echo Pan"},
    {127, 44, "MT32 DoctorSolo"}, {127, 45, "MT32 SchoolDaze"},  {127, 46, "MT32 Bellsinger"}, {127, 47, "MT32 SquareWave"},
    {127, 48, "MT32 Str Sect 1"}, {127, 49, "MT32 Str Sect 2"},  {127, 50, "MT32 Str Sect 3"}, {127, 51, "MT32 Pizzicato"},
    {127, 52, "MT32 Violin 1"},   {127, 53, "MT32 Violin 2"},    {127, 54, "MT32 Cello 1"},    {127, 55, "MT32 Cello 2"},
    {127, 56, "MT32 Contrabass"}, {127, 57, "MT32 Harp 1"},      {127, 58, "MT32 Harp 2"},     {127, 59, "MT32 Guitar 1"},
    {127, 60, "MT32 Guitar 2"},   {127, 61, "MT32 Elec Gtr 1"},  {127, 62, "MT32 Elec Gtr 2"}, {127, 63, "MT32 Sitar"},
    {127, 64, "MT32 Acou Bass1"}, {127, 65, "MT32 Acou Bass2"},  {127, 66, "MT32 Elec Bass1"}, {127, 67, "MT32 Elec Bass2"},
    {127, 68, "MT32 Slap Bass1"}, {127, 69, "MT32 Slap Bass2"},  {127, 70, "MT32 Fretless 1"}, {127, 71, "MT32 Fretless 2"},
    {127, 72, "MT32 Flute 1"},    {127, 73, "MT32 Flute 2"},     {127, 74, "MT32 Piccolo 1"},  {127, 75, "MT32 Piccolo 2"},
    {127, 76, "MT32 Recorder"},   {127, 77, "MT32 Pan Pipes"},   {127, 78, "MT32 Sax 1"},      {127, 79, "MT32 Sax 2"},
    {127, 80, "MT32 Sax 3"},      {127, 81, "MT32 Sax 4"},       {127, 82, "MT32 Clarinet 1"}, {127, 83, "MT32 Clarinet 2"},
    {127, 84, "MT32 Oboe"},       {127, 85, "MT32 Engl Horn"},   {127, 86, "MT32 Bassoon"},    {127, 87, "MT32 Harmonica"},
    {127, 88, "MT32 Trumpet 1"},  {127, 89, "MT32 Trumpet 2"},   {127, 90, "MT32 Trombone 1"}, {127, 91, "MT32 Trombone 2"},
    {127, 92, "MT32 Fr Horn 1"},  {127, 93, "MT32 Fr Horn 2"},   {127, 94, "MT32 Tuba"},       {127, 95, "MT32 Brs Sect 1"},
    {127, 96, "MT32 Brs Sect 2"}, {127, 97, "MT32 Vibe 1"},      {127, 98, "MT32 Vibe 2"},     {127, 99, "MT32 Syn Mallet"},
    {127, 100, "MT32 Windbell"},  {127, 101, "MT32 Glock"},      {127, 102, "MT32 Tube Bell"}, {127, 103, "MT32 Xylophone"},
    {127, 104, "MT32 Marimba"},   {127, 105, "MT32 Koto"},       {127, 106, "MT32 Sho"},       {127, 107, "MT32 Shakuhachi"},
    {127, 108, "MT32 Whistle 1"}, {127, 109, "MT32 Whistle 2"},  {127, 110, "MT32 Bottleblow"},{127, 111, "MT32 Breathpipe"},
    {127, 112, "MT32 Timpani"},   {127, 113, "MT32 MelodicTom"}, {127, 114, "MT32 Deep Snare"},{127, 115, "MT32 Elec Perc1"},
    {127, 116, "MT32 Elec Perc2"},{127, 117, "MT32 Taiko"},      {127, 118, "MT32 Taiko Rim"}, {127, 119, "MT32 Cymbal"},
    {127, 120, "MT32 Castanets"}, {127, 121, "MT32 Triangle"},   {127, 122, "MT32 Orche Hit"}, {127, 123, "MT32 Telephone"},
    {127, 124, "MT32 Bird 2"},    {127, 125, "MT32 1 Note Jam"}, {127, 126, "MT32 Water Bell"},{127, 127, "MT32 JungleTune"}
};
// Roland GS Percussion Sets Table (Ch 10 Exclusive Map) - [10 Sets]
const InstrumentAllocation scb55_drum_kits[10] PROGMEM = {
    {0, 0,  "Standard Set"},
    {0, 8,  "Room Set"},
    {0, 16, "Power Set"},
    {0, 24, "Electronic Set"},
    {0, 25, "TR-808 Set"},
    {0, 32, "Jazz Set"},
    {0, 40, "Brush Set"},
    {0, 48, "Orchestra Set"},
    {0, 56, "SFX Set"},
    {0, 127, "CM-64/32L Set"}
};

// =========================================================================
// 3. GLOBAL UI STATE MANAGEMENT VARIABLES
// =========================================================================
enum MenuMode { BROWSE_PATCH, SET_CHANNEL, MAIN_SYSTEM_MENU, SYSEX_RESETS_MENU, FX_SETTINGS_MENU, CHAN_FX_MENU };
uint8_t currentMode = SET_CHANNEL; 

int cursorIndex = 0;          
int activeIndex = 0;          
uint8_t currentChannel = 1;   
uint8_t lastChannel = 0;   //for display control of Channel / Patch screen
uint8_t currentVolume = 0;    
int lastAnalogValue = 0; 
uint16_t lastAnalogPoll = 0;
char message[21]; 

//Serial1 SysEx Handling
#define MIDI_SYSEX_START 0xF0
#define MIDI_SYSEX_END   0xF7
#define MAX_SYSEX_BUF    80 // Safeguard against buffer overflows
uint8_t sysExBuffer[MAX_SYSEX_BUF];
uint8_t sysExIndex = 0;
bool isRecordingSysEx = false;

//USB MIDI TX Buffer Status
// Safe, non-blocking check for Teensy 2.0 MIDI hardware buffer readiness
#define USB_MIDI_READY ([]() -> bool { \
    uint8_t current_endpoint = UENUM; \
    UENUM = 4; \
    uint16_t timeout = 600; \
    while (!(UEINTX & (1 << RWAL))) { \
        if (--timeout == 0) { \
            UENUM = current_endpoint; \
            return false; /* Host is not listening (Power-only mode) */ \
            break; \
        } \
    } \
    UENUM = current_endpoint; \
    return true; /* Buffer is free to accept data */ \
}())

// Screen Refresh Limiter for UI
const uint16_t refreshRateMaxMs = 666; // 333mS = 3 updates a second
uint16_t lastScreenUpdateTime = 0;

// Non-blocking screen saver configurations
unsigned long lastMidiMessageTime = 0;
const unsigned long messageTimeoutMs = 5000; // 5 seconds timeout limit
bool isDisplayActive = false;
bool activity = true; //activity from USB or UI
bool screenOn = true;
uint16_t screenSaver = 0;

int8_t systemMenuSelection = 0;  
int8_t sysExSelection = 0;       
int8_t fxSelectedParam = 0;      
bool isEditingParam = false; 
bool inChannelMenu = true; //For saving menu redraw, resused from Channel Memu, should prob change to inMenu

// Global FX values
uint8_t masterVolumeState = 127; //Setup init values track SC-55 MkII (with screen)
uint8_t masterReverbState = 64;
uint8_t masterChorusState = 64;

// Per-Channel UI tracking arrays
uint8_t chanVolume[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t chanReverb[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t chanChorus[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Multi-Channel Patch Tracking Maps (Stores active array indexes)
int chanActivePatch[16]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Current Bank/Program index#
uint8_t chanCurrentBank[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Raw read Bank
uint16_t channelMap = 0x0200; // 16 bits: 0 = Instrument, 1 = Drum (If channel set to drum map) default ch10

uint8_t chanFxChannel  = 1;
int8_t chanFxSelectedParam = 0;
bool parserIsIdle = true;
bool updateDisplayFlag = true;
bool volumeAnalogDisplay = true;
bool fallback = false; //CTF Tracker

// Debounce & timing variables
uint8_t lastButtonReading = HIGH;     
uint16_t lastDebounceTime = 0;
const uint16_t debounceDelay = 40;        

uint8_t stableButtonState = HIGH;      
uint8_t previousStableButtonState = HIGH;

uint16_t buttonPressTime = 0;             
const uint16_t longPressThreshold = 800;  

long lastEncoderTicks = 0;
uint16_t lastClickTime = 0;               
uint8_t clickCount = 0;                             
const uint16_t doubleClickThreshold = 280; 
bool ignoreNextRelease = false;                 

uint16_t lastEncoderMoveTime = 0;         

// =========================================================================
// 4. SYSEX ARRAY PAYLOADS
// =========================================================================
const byte gsReset[]   = {0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7};
const byte gmReset[]   = {0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7};
//const byte mt32Reset[] = {0xF0, 0x41, 0x10, 0x16, 0x12, 0x7F, 0x01, 0xF7}; //This is an MT32 code 0x16
const uint8_t mt32EmuPatches[16] = {0, 68, 48, 95, 78, 41, 3, 110, 122, 127, 0, 0, 0, 0, 0, 0};
const uint8_t mt32EmuPanFix[10] = {64, 74, 74, 74, 74, 110, 37, 127, 1, 64}; //L&R Swapped on SC-55 

// =========================================================================
// 5. FLASH READ & LOOKUP UTILITIES
// =========================================================================
InstrumentAllocation fetchInstrumentFromFlash(int index) {
    InstrumentAllocation targetRecord;
    targetRecord.bank = pgm_read_byte(&(scb55_bank[index].bank));
    targetRecord.program = pgm_read_byte(&(scb55_bank[index].program));
    memcpy_P(targetRecord.name, scb55_bank[index].name, sizeof(targetRecord.name));
    return targetRecord;
}

InstrumentAllocation fetchDrumKitFromFlash(int index) {
    InstrumentAllocation targetRecord;
    targetRecord.bank = pgm_read_byte(&(scb55_drum_kits[index].bank));
    targetRecord.program = pgm_read_byte(&(scb55_drum_kits[index].program));
    memcpy_P(targetRecord.name, scb55_drum_kits[index].name, sizeof(targetRecord.name));
    return targetRecord;
}

// ==========================================================================
// 5a. QUICKLY LOOK UP IF CHANNEL IS SET TO PATCH OR DRUM KIT (map 1&2 = on)
// ==========================================================================
bool isDrumPatch(uint8_t channel) { // Channel 1 to 16
    // 1U << channel tells the AVR compiler to use its 
    // smallest built-in bit-shifting routine.
    return channelMap & (1U << ((channel - 1) & 0x0F));
}

// ==========================================================================
// 5b. QUICKLY LOOK UP NAME FOR INCOMING PATCH / DRUM KIT + CT FALLBACK
// ==========================================================================
void synchronizeUIWithIncomingPatch(uint8_t channel, uint8_t bank, uint8_t program) {
    fallback = false; 
    if (isDrumPatch(channel)) {
        // Drum Channel tracks against the 10 Drum Kit options
        
        if (program > 63 && program < 127) { // Invalid patches fallback to 0 (Standard)
        program = 0; 
        fallback = true; 
        } else if (program != 25 && program != 127) { // The two valid exceptions
            if ((program & 0x07) != 0) {  // If we have to round patch down
                program &= 0xF8; // Map down to next valid patch
                fallback = true; 
            }
        } // We have a valid drum patch!

        for (int i = 0; i < 10; i++) {
            uint8_t p = pgm_read_byte(&(scb55_drum_kits[i].program));
            if (p == program) {
                chanActivePatch[channel - 1] = i; 
                if (isDrumPatch(currentChannel)) {
                    cursorIndex = i;
                    activeIndex = i;
                }
                updateDisplayFlag = true;
                break;
            }
        }
    } else {
        retry: //bank set to zero for auto-fallback now find patch in bank 0
            // Channels 1-9 & 11-16 track against 354 melodic presets (binary search)
            int low = 0;
            int high = 353; // 354 elements total (0 to 353)
            int foundIndex = -1;

            while (low <= high) {
                int i = low + (high - low) / 2;
    
                    uint8_t b = pgm_read_byte(&(scb55_bank[i].bank));
                    uint8_t p = pgm_read_byte(&(scb55_bank[i].program));

                    if (b == bank && p == program) {
                        foundIndex = i;
                        break; // Precise match found!
                    }

                    // Sort evaluation: Check bank first. If banks match, check program.
                    if (b < bank || (b == bank && p < program)) {
                            low = i + 1;  // Search upper half
                    } else {
                            high = i - 1; // Search lower half
                        }
                }

                    // Update layout if a match was successfully found
                    if (foundIndex != -1) {  // In fallback mode this should always be true
                    chanActivePatch[channel - 1] = foundIndex; 

                        if (channel == currentChannel) {
                            cursorIndex = foundIndex;
                            activeIndex = foundIndex;
                        }
                    updateDisplayFlag = true;
                    } else { if (bank < 126 || program < 120) {bank = 0; fallback = true; goto retry;} }
        }         
                    if (fallback) { sendMidiControlChange(channel, 0, 0); // Bank fallback to 0
                                    sendMidiProgramChange(channel, program); // Send corrected patch
                                    fallback = false; }
        
}

// ==========================================================================
// 5c. REPLACE delay(); WITH _delay_ms (delay.h) FOR SPACE SAVING
// ==========================================================================
static void tinyDelay(uint16_t ms) {
    while (ms--) {
        _delay_ms(1); // Tiny 1ms loop - large delays take more code space
    }
}

// ===========================================================================
// 6. MIDI TRANSMISSION UTILITIES (DUAL SERIAL + USB ROUTING + SYSEX HANDLING)
// ===========================================================================
void sendMidiControlChange(uint8_t channel, uint8_t control, uint8_t value) {
    Serial1.write(0xB0 | ((channel - 1) & 0x0F));
    Serial1.write(control & 0x7F);
    Serial1.write(value & 0x7F);
    if (!fallback && USB_MIDI_READY) usbMIDI.sendControlChange(control, value, channel); // Echo back
}

void sendMidiProgramChange(uint8_t channel, uint8_t program) {
    Serial1.write(0xC0 | ((channel - 1) & 0x0F));
    Serial1.write(program & 0x7F);
    if (!fallback && USB_MIDI_READY) usbMIDI.sendProgramChange(program, channel); // Echo back
}

void commitPatchChange(int index) {
    if (isDrumPatch(currentChannel)) {
        InstrumentAllocation kit = fetchDrumKitFromFlash(index);
        sendMidiControlChange(10, 0, kit.bank);
        sendMidiProgramChange(10, kit.program);
        activeIndex = index;
        chanActivePatch[currentChannel - 1] = index;
        chanCurrentBank[currentChannel - 1] = kit.bank; //add for dynamic drum channel
    } else {
        InstrumentAllocation patch = fetchInstrumentFromFlash(index);
        sendMidiControlChange(currentChannel, 0, patch.bank);
        sendMidiProgramChange(currentChannel, patch.program);
        activeIndex = index;
        chanActivePatch[currentChannel - 1] = index; 
        chanCurrentBank[currentChannel - 1] = patch.bank;
    }
}

void sendRawSysEx(const byte* array, byte size) {
    Serial1.write(array, size);
    if (USB_MIDI_READY) usbMIDI.sendSysEx(size, array, true);
}

void sendGSSysEx(uint8_t addrMSB, uint8_t addrMD, uint8_t addrLSB, uint8_t value) {
    uint32_t sum = addrMSB + addrMD + addrLSB + value;
    uint8_t checksum = (128 - (sum % 128)) & 0x7F;

    uint8_t sysexMsg[] = {
        0xF0, 0x41, 0x10, 0x42, 0x12, 
        addrMSB, addrMD, addrLSB, 
        value, checksum, 0xF7
    };
    Serial1.write(sysexMsg, sizeof(sysexMsg)); 
    if (USB_MIDI_READY) usbMIDI.sendSysEx(sizeof(sysexMsg), sysexMsg, true);
}

void triggerMidiPanic() {
    display.clearDisplay();
    display.setTextSize(2);
    //display.setTextColor(SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.setCursor(9, 15);
    display.print(F("!!PANIC!!"));
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(5, 45);
    display.print(F("Clearing 16 Chans..."));
    display.display();

    for (uint8_t ch = 1; ch <= 16; ch++) {
        sendMidiControlChange(ch, 123, 0); 
        sendMidiControlChange(ch, 120, 0); 
    }
    tinyDelay(600); 
    updateDisplayFlag = true;   //Set to re-draw non-static parts of displayed menu
    inChannelMenu = true;       //Set to re-draw semi-static parts of displayed menu
    drawStaticUIFrame();        //Re-draw static parts of display
}

void executeSysExTransfer(uint8_t selection) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 15);
    display.print(F("Sending SysEx..."));
    display.setCursor(0, 35);
    
    switch(selection) {
        case 0:
            display.print(F(">> GS Mode Reset"));
            sendRawSysEx(gsReset, sizeof(gsReset));
            break;
        case 1:
            display.print(F(">> GM Mode Reset"));
            sendRawSysEx(gmReset, sizeof(gmReset));
            break;
        case 2:
            display.print(F(">> MT-32 Emulation"));
            //sendRawSysEx(mt32Reset, sizeof(mt32Reset)); //Not a real MT-32
            sendRawSysEx(gsReset, sizeof(gsReset)); //So we can enable MT-32 emulation
            break;
    } // Fall through for received reset SysEx (selection == 4)
    
    if (selection != 4) { display.display(); tinyDelay(500); }

    int targetIndex = 0;
    if (selection == 2) { 
        for (int i = 0; i < 354; i++) { //Just for a fallback if patch in list not found
            if (pgm_read_byte(&(scb55_bank[i].bank)) == 127) {
                targetIndex = i;
                break;
            }
        }
    } else { 
        for (int i = 0; i < 354; i++) { //This should be 0, but just in case
            if (pgm_read_byte(&(scb55_bank[i].bank)) == 0) {
                targetIndex = i;
                break;
            }
        }
    }
    
    for(uint8_t ch=0; ch<16; ch++) {
            chanActivePatch[ch] = targetIndex;
            chanCurrentBank[ch] = 0; // For GS & GM reset & MT-32 Fallback in case something goes wrong
        if (selection == 2) synchronizeUIWithIncomingPatch((ch + 1), 127, mt32EmuPatches[ch]);
        cursorIndex = chanActivePatch[ch]; //Push patch & bank changes to SCB-55 after reset
        currentChannel = (ch + 1);
        if (selection != 4) commitPatchChange(cursorIndex); 
        //chanVolume[ch] = 100; //Reset UI Vol, Rev, Chor to default
        //chanReverb[ch] = 40;
        //chanChorus[ch] = 0;
    }
        for (uint8_t ch=0; ch<10; ch++) {
            if (selection == 2) {
            sendGSSysEx(0x40, (ch | 0x20), 0x10, 0x4C); //Change pitch bend for MT-32 emu
            sendMidiControlChange(ch+1, 91, 64); //Set Reverb (CC91) for MT-32 emu
            sendMidiControlChange(ch+1, 10, mt32EmuPanFix[ch]); //Set FixedPan (CC10) for MT-32 emu
        } //else { if (selection !=4) sendGSSysEx(0x40, (ch | 0x20), 0x10, 0x42);}} //Reset pitch bend to default
        } //Above not needed as GS/GM reset has already done this

    channelMap = (1U << 9); //Reset drum part to channel 10 only
    memset(chanVolume, 100, sizeof(chanVolume));        //Reset UI Vol, Rev, Chor to default 
    memset(chanReverb,  40, sizeof(chanReverb));        //This uses 32 less program space 
    memset(chanChorus,   0, sizeof(chanChorus));        //than chanX[ch]=x as above.
    if (selection == 2) { memset(chanReverb,  64, 10); //Set UI channel 1 - 10 Rev for MT-32 emu
        sendMidiControlChange(10, 11, 80); } //Set Expression (CC11) for MT-32 emu drums
    currentChannel = 1; //Put UI on channel 1
}

// =========================================================================
// 6a. Serial MIDI SysEx handling
// =========================================================================
void processSysEx(const uint8_t type) {
    Serial1.write(type); //Push out bytes as we get them (prevents SysEx > 80 = stream corruption)

    if (type == MIDI_SYSEX_START) { 
        isRecordingSysEx = true;
        sysExIndex = 0;
        sysExBuffer[sysExIndex++] = MIDI_SYSEX_START; 
        return; //Exit and wait for next byte
    }

    if (isRecordingSysEx) {
        // Collect the incoming stream byte
        if ((type >= 0x80) && (type != MIDI_SYSEX_END)) return; //Bad Byte!
        if (sysExIndex < MAX_SYSEX_BUF) {
            sysExBuffer[sysExIndex++] = type; 
        } else {
            isRecordingSysEx = false; // Buffer overflow safety abort
            return; //Abort
        }

        // Check if we hit the end of the SysEx transmission
        if (type == MIDI_SYSEX_END) { 
            isRecordingSysEx = false; //End of SysEx, so stop
            uint8_t sysExLen = sysExIndex; // Total byte footprint
            sysExIndex = 0;

            //  Optimized 8-bit filter execution
            if (sysExLen > 9 && sysExBuffer[1] == 0x41 && sysExBuffer[4] == 0x12) {
                uint8_t d3 = sysExBuffer[3]; //Model
                uint8_t d5 = sysExBuffer[5]; //High Address
                uint8_t d6 = sysExBuffer[6]; //Mid Address

                if (sysExLen == 74) {
                    // Flat 8-bit identity verification
                    if (d5 == 0x10 && d6 == 0x01 && sysExBuffer[7] == 0x00) {
                        injectPayloadToDisplayBuffer(sysExBuffer + 8); return;
                    }
                } else {
                    // Flat text line routing verification
                    bool isTextLine = (d3 == 0x16 && d5 == 0x20) || 
                                      (d3 == 0x45 && d5 == 0x10 && d6 == 0x00);
                    if (isTextLine) {
                        sysExTextLine(sysExBuffer, sysExLen); return;
                    } else {
                        if (d3 == 0x42 && d5 == 0x40) {
                            if (sysExBuffer[7] == 0x15 && (d6 & 0xF0) == 0x10) { //Set Drum Map
                                uint8_t ch = (d6 & 0x0F); ch = (ch == 0) ? 10 : (ch > 9) ? ch + 1 : ch;
                                channelMap = (sysExBuffer[8] > 0) ? (channelMap | (1U << (ch - 1))) : (channelMap & ~(1U << (ch - 1))); return;}
                            if (d6 == 0x00 && sysExBuffer[7] == 0x7F && sysExBuffer[8] == 0x00) {
                                inboundReset(); return;} //gsReset received!
                        }
                    }
                }
            } else { // <10
                if (sysExBuffer[1] == 0x7E && sysExBuffer[2] == 0x7F && sysExBuffer[3] == 0x09 && sysExBuffer[4] == 0x01) {
                inboundReset(); return; //gmReset received!
                }
            }
        }
    }
    //empty
}
// =========================================================================
// 6b. INCOMMING RESET HANDLER
// =========================================================================
void inboundReset() {
    executeSysExTransfer(4); //Special mode so we can reuse menu code
    updateDisplayFlag = true;//
    inChannelMenu = true;    //Redraw full screen after reset
    drawStaticUIFrame();     //
}

// =========================================================================
// 7a. OLED DRAW UI SUB PIPELINE (SETUP STATIC ELEMENTS)
// =========================================================================
void drawStaticUIFrame() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // 1. Permanent Layout Headings
    display.setCursor(0, 0);
    display.print(F("ROLAND SCB-55cw"));
    display.setCursor(98, 0);
    display.print(F("CH:"));
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    // 2. Permanent Footer Frames (Only for standard playback modes)
    if (currentMode == BROWSE_PATCH || currentMode == SET_CHANNEL) {
        display.drawFastHLine(0, 55, 128, SSD1306_WHITE);
        display.setCursor(0, 57);
        display.print(F("ACT:"));
        display.setCursor(85, 57);
        display.print(F("VOL:"));
        volumeAnalogDisplay = true; //redraw Volume value
        lastChannel=0; //clear to redraw middle main screen text & channel # at top
    }
    
    // Blast the foundational structure to the screen once (blocking is fine here)
    display.display();
}

// =========================================================================
// 7b. OLED DRAW UI PIPELINE (DRAW SCREEN IN PARTS FOR SPEED)
// =========================================================================
void renderInterfaceUI() {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Conditionally fetch records based on active channel selection
    InstrumentAllocation cursorPatch = (isDrumPatch(currentChannel)) ? fetchDrumKitFromFlash(cursorIndex) : fetchInstrumentFromFlash(cursorIndex);
    InstrumentAllocation activePatch = (isDrumPatch(currentChannel)) ? fetchDrumKitFromFlash(activeIndex) : fetchInstrumentFromFlash(activeIndex);

    // =========================================================================
    // ZONE 1: TOP RIGHT HAND HEADER TRACKER (CH NUMBER)
    // =========================================================================
    if ((currentMode == SET_CHANNEL || currentMode == BROWSE_PATCH) && (currentChannel != lastChannel)) { 
        display.fillRect(114, 0, 14, 8, SSD1306_BLACK); // Micro-clear old number digits
        display.setCursor(114, 0);
        display.print(currentChannel);
        displayPartialHorizontalWindow(114, 0, 11, 1);            // Sync the corner box
     }

    // =========================================================================
    // ZONE 2: MIDDLE DATA WINDOW BLOCK (ROWS 2 TO 6)
    // =========================================================================
    // Clear only the workspace core area, leaving static borders completely untouched
 
    if (currentMode == SET_CHANNEL) {
        if (currentChannel != lastChannel) { 
            display.fillRect(0, 16, 107, 8, SSD1306_BLACK); //Erase text above Patch if channel change
            //lastChannel = currentChannel; we do this later
        }
        display.fillRect(35, 32, 89, 8, SSD1306_BLACK); //Erase patch name
    } else {
   
    if (currentMode == BROWSE_PATCH) {
        if (inChannelMenu) { 
            display.fillRect(0, 16, 107, 8, SSD1306_BLACK); //Erase text above Patch if menu change
            } //inChannelMenu = false; we do this later
            display.fillRect(10, 28, 89, 19, SSD1306_BLACK); //Erase patch info numbers and name
        
        } else { 
    if (currentMode == CHAN_FX_MENU) {
        display.fillRect(0, 11, 125, 50, SSD1306_BLACK); //Larger clear needed for FX Menu
    } else { //Inner area (0,11 to 112,61) clear for all other menus
        display.fillRect(0, 11, 112, 50, SSD1306_BLACK); 
    }   }
    } 

    switch (currentMode) {
        case SET_CHANNEL: 
            display.setCursor(0, 16);
            display.print(isDrumPatch(currentChannel) ? F("> Change Ch (DRUM)") : F("> Change Channel"));
            display.setCursor(5, 32);
            display.print(F("Act: ")); display.print(activePatch.name);
            display.setCursor(1, 44);
            display.print(message);
            break;

        case BROWSE_PATCH: {
            int maxPatches = (isDrumPatch(currentChannel)) ? 10 : 354;
            display.setCursor(0, 16);
            display.print(isDrumPatch(currentChannel) ? F("> Scroll Drum Kits") : F("> Scroll Patches"));
            display.setCursor(10, 28);
            display.print(cursorIndex + 1); display.print(F("/")); display.print(maxPatches);
            display.print(F(" P:")); display.print(cursorPatch.program);
            display.setCursor(10, 39);
            display.print(cursorPatch.name);
            break;
        }

        case MAIN_SYSTEM_MENU:
            display.setCursor(7, 14);
            display.print(F("--- SYSTEM MENU ---"));
            display.setCursor(10, 24);
            display.print(systemMenuSelection == 0 ? F("-> SysEx Resets") : F("   SysEx Resets"));
            display.setCursor(10, 34);
            display.print(systemMenuSelection == 1 ? F("-> Global FX") : F("   Global FX"));
            display.setCursor(10, 44);
            display.print(systemMenuSelection == 2 ? F("-> Channel Mixer") : F("   Channel Mixer"));
            display.setCursor(10, 54);
            display.print(systemMenuSelection == 3 ? F("-> Exit Menu") : F("   Exit Menu"));
            break;

        case SYSEX_RESETS_MENU:
            display.setCursor(4, 14);
            display.print(F("--- SYSEX RESETS ---"));
            display.setCursor(10, 24);
            display.print(sysExSelection == 0 ? F("-> GS Reset") : F("   GS Reset"));
            display.setCursor(10, 34);
            display.print(sysExSelection == 1 ? F("-> GM Reset") : F("   GM Reset"));
            display.setCursor(10, 44);
            display.print(sysExSelection == 2 ? F("-> MT-32 Mode") : F("   MT-32 Mode"));
            display.setCursor(10, 54);
            display.print(sysExSelection == 3 ? F("-> [ Back ]") : F("   [ Back ]"));
             break;

        case FX_SETTINGS_MENU:
            display.setCursor(4, 14);
            display.print(F("-- GLOBAL EFFECTS --"));
            
            display.setCursor(5, 24);
            display.print((fxSelectedParam == 0) ? (isEditingParam ? F(" * ") : F("-> ")) : F("   "));
            display.print(F("Master Vol: ")); display.print(masterVolumeState);
            
            display.setCursor(5, 34);
            display.print((fxSelectedParam == 1) ? (isEditingParam ? F(" * ") : F("-> ")) : F("   "));
            display.print(F("Reverb Lvl: ")); display.print(masterReverbState);
            
            display.setCursor(5, 44);
            display.print((fxSelectedParam == 2) ? (isEditingParam ? F(" * ") : F("-> ")) : F("   "));
            display.print(F("Chorus Lvl: ")); display.print(masterChorusState);
            
            display.setCursor(5, 54);
            display.print((fxSelectedParam == 3) ? F("-> ") : F("   "));
            display.print(F("[ Back ]"));
            break;

        case CHAN_FX_MENU: {
            //display.setCursor(0, 2);
            //display.print(F("--- CHANNEL MIXER ---"));
            
            display.setCursor(5, 14);
            display.print((chanFxSelectedParam == 0) ? (isEditingParam ? F("* ") : F("-> ")) : F("   "));
            display.print(F("MIDI Chan:  CH ")); display.print(chanFxChannel);
            
            // --- VOLUME BAR (Row 24 / Page 3) ---
            display.setCursor(5, 24);
            display.print((chanFxSelectedParam == 1) ? (isEditingParam ? F("* ") : F("-> ")) : F("   "));
            display.print(F("Vol: ")); 
            uint8_t vVal = chanVolume[chanFxChannel - 1]; display.print(vVal);
            drawDirectBar(3, vVal); // 3 = Page 3 (Y:24)
            
            // --- REVERB BAR (Row 34 / Page 4) ---
            display.setCursor(5, 34);
            display.print((chanFxSelectedParam == 2) ? (isEditingParam ? F("* ") : F("-> ")) : F("   "));
            display.print(F("Rev: ")); 
            uint8_t rVal = chanReverb[chanFxChannel - 1]; display.print(rVal);
            drawDirectBar(4, rVal); // 4 = Page 4 (Y:32-39, closely covers Y:34)
            
            // --- CHORUS BAR (Row 44 / Page 5) ---
            display.setCursor(5, 44);
            display.print((chanFxSelectedParam == 3) ? (isEditingParam ? F("* ") : F("-> ")) : F("   "));
            display.print(F("Cho: ")); 
            uint8_t cVal = chanChorus[chanFxChannel - 1]; display.print(cVal);
            drawDirectBar(5, cVal); // 5 = Page 5 (Y:40-47, closely covers Y:44)
            
            display.setCursor(5, 54);
            display.print((chanFxSelectedParam == 4) ? F("-> ") : F("   "));
            display.print(F("[ Back ]"));
            break;
        }

    }
    // Update the central content zone (Columns 0-110, Rows Pages 1 to 6)
    if (currentMode == SET_CHANNEL) {
        if (currentChannel != lastChannel) { 
            displayPartialHorizontalWindow(0, 2, 107, 1); //Refresh text above Patch
            displayPartialHorizontalWindow(0, 4, 35, 1); //Refresh text left of Patch
            lastChannel = currentChannel;
        }
        displayPartialHorizontalWindow(35, 4, 89, 1); //Draw patch name
        if (isDisplayActive == true) {
            displayPartialHorizontalWindow(0, 5, 128, 2); //Draw sysEx Text Message
        }
    } else {
    if (currentMode == BROWSE_PATCH) {
        if (inChannelMenu) {
        displayPartialHorizontalWindow(0, 2, 107, 1); //Refresh text above Patch Info
        inChannelMenu = false; }                      //Reset so text draws only once in this menu
        displayPartialHorizontalWindow(10, 3, 98, 3); //Draw all patch info (Name, ###/###, & P:###)
    } else { if (currentMode == CHAN_FX_MENU) {
        displayPartialHorizontalWindow(0, 1, 125, 7); //Draw complete Channel FX Menu
            } else { 
                if (inChannelMenu) { //For Most menus with a title
                displayPartialHorizontalWindow(0, 1, 112, 1); //Part of the top line
                displayPartialHorizontalWindow(0, 2, 125, 1); //This part of the top line is longer
                displayPartialHorizontalWindow(0, 3, 112, 5); //The rest of the menu
                inChannelMenu = false; //reset so text only draws once in this menu
                } else {
            if (currentMode == FX_SETTINGS_MENU) { //Has two columns to draw
                displayPartialHorizontalWindow(5, 3, 20, 5); //Redraw the cursor
                displayPartialHorizontalWindow(95, 3, 17, 5); //Redraw the Data
            } else { //The -> cursor for all other menus
                    displayPartialHorizontalWindow(10, 3, 16, 5); //Just redraw the cursor
                        }
                }
                    }   
            }   
    }

    // =========================================================================
    // ZONE 3: LOWER FOOTER VALUE BAR (PIXELS 57 TO 64)
    // =========================================================================
    if (currentMode == BROWSE_PATCH || currentMode == SET_CHANNEL) {
        // Clear active patch numbers zone
        display.fillRect(24, 57, 56, 7, SSD1306_BLACK);
        display.setCursor(24, 57);
        display.print(activePatch.program); 
        if (activePatch.bank!=chanCurrentBank[currentChannel - 1]) display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  
        display.print(F(" B:")); 
        //display.print(activePatch.bank); //Shows valid banks only
        display.print(chanCurrentBank[currentChannel - 1]); //Show valid and invalid bank
        display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

        // Clear active master feedback meter numbers zone
        display.fillRect(110, 57, 18, 7, SSD1306_BLACK); 
        display.setCursor(110, 57);
        display.print(currentVolume);

        // BLAST ZONE 3: 
        // Zone A: Active Patch Values (X:24, 60px wide, Row Page 7)
        displayPartialHorizontalWindow(24, 7, 54, 1);
        
        // Zone B: Master Volume Numerical Feedback (X:110, 18px wide, Row Page 7) Update on change
        if (volumeAnalogDisplay) displayPartialHorizontalWindow(110, 7, 17, 1); volumeAnalogDisplay = false;
    } 
}

// =========================================================================
// 7c. SINGLE LINE SYSEX TEXT DISPLAY - 20 CHARACTERS
// =========================================================================
void sysExTextLine(const byte *sysExData,  unsigned int sysExLen) {
    clearTextDisplayWindow();
    //memcpy(message, sysExData + 8, (min(sysExLen, 30) - 10));      //Limit check
    //message[(min(sysExLen, 30) - 10)] = '\0';  //Macro uses 2 Program Bytes more than math below
    sysExLen = sysExLen - 10;         //Length adjust and limit check (The easy way!)
    if (sysExLen > 20) sysExLen = 20; //This saves 6 bytes over our math below!
    memcpy(message, sysExData + 8, sysExLen); //Copy SysEx message to message variable
    message[sysExLen] = '\0';                 //Null terminate string variable
    //unsigned int length = ((sysExLen + 10) - (sysExLen > 30 ? (sysExLen - 30) : (30 - sysExLen))) / 2;//Limit check
    //memcpy(message, sysExData + 8, length); // Saves 2 bytes over min macro
    //message[length] = '\0'; //8 Program Bytes
    lastMidiMessageTime = millis();   // Reset the activity timer tracking metrics
    isDisplayActive = true;
    updateDisplayFlag = true;
}

// =========================================================================
// 7d. BITMAP MANIPULATION FOR 16x16 SYSEX DISPLAY
// =========================================================================
// Highly optimized, zero-SRAM translator that injects bits directly into the library's buffer
void injectPayloadToDisplayBuffer(const uint8_t* sc55Payload) {
  // Grab a direct pointer to the Adafruit library's internal live pixel frame buffer
  uint8_t* screenBuffer = display.getBuffer();
  //if (!screenBuffer) return; //Probably not needed

  // Process through the 4 hardware memory blocks
  for (int block = 0; block < 4; ++block) {
    for (int line = 0; line < 16; ++line) {
      
      uint8_t dataByte = sc55Payload[(block * 16) + line];

      // Extract the lower 5 bits (Bit 4 down to Bit 0)
      for (int bit = 0; bit < 5; ++bit) {
        int targetPixelX = (block * 5) + bit;
        
        // Enforce the strict 16-pixel width boundary limit
        if (targetPixelX < 16) {
          bool isPixelOn = (dataByte & (1 << (4 - bit))) != 0;
          
          // Calculate the target pixel's absolute 1D index inside the library's screen array
          int finalPixelY = 16 + line; // Display at X:112 Y:32
          int bufferIndex = (finalPixelY / 8) * SCREEN_WIDTH + (112 + targetPixelX);
          uint8_t bitMask = (1 << (finalPixelY % 8));

          // Directly read/write from/to the library canvas memory pointer in-place
          if (isPixelOn) {
            screenBuffer[bufferIndex] |= bitMask;  // Set pixel white
          } else {
            screenBuffer[bufferIndex] &= ~bitMask; // Set pixel black
          }
        }
      }
    }
  }

  // Update only the 16x16 icon block space (X:112, Row Page 2, 16px wide, 2 pages high)
  // Push the freshly modified internal buffer chunk straight to the physical hardware
  displayPartialHorizontalWindow(112, 2, 16, 2); 

    // Reset the activity timer tracking metrics
    lastMidiMessageTime = millis();
    isDisplayActive = true;
    activity = true;
}

// =========================================================================
// 7e. BITMAP CLEAR FOR 16x16 SYSEX DISPLAY & SYSEX TEXT LINE
// =========================================================================
// Low-RAM helper function to wipe just the 16x16 display window region cleanly
void clearTextDisplayWindow() {
    /*for (uint8_t line = 0; line < 16; ++line) {
        for (uint8_t x = 0; x < 16; ++x) {
        display.drawPixel(112 + x, 16 + line, SSD1306_BLACK); // Display at X:112 Y:32
        }
    } */
    display.fillRect(112, 16, 16, 16, SSD1306_BLACK); // Instead of the above loop? (save 26 bytes)
    displayPartialHorizontalWindow(112, 2, 16, 2); 
    //memcpy(message, F("                    "), 20); message[20] = '\0'; //Reset SysEx Message text
    memcpy(message, F(" "), 1); message[1] = '\0';  //Reset SysEx Message text
    display.fillRect(0, 44, 128, 8, SSD1306_BLACK); //Erase sysEx Text Message
    displayPartialHorizontalWindow(0, 5, 128, 2); 
    isDisplayActive = false;
}

// =========================================================================
// 7f. BAR BITMAP DRAW FOR LEVELS (Speed up)
// =========================================================================
void drawDirectBar(uint8_t page, uint8_t rawValue) {
  // 1. Unroll the map logic into lean integer arithmetic (0 to 33 filled pixels max)
  uint8_t filledWidth = (rawValue * 33) / 127;
  
  // 2. Locate Adafruit's internal screen pointer array (128 columns per page)
  uint16_t bufferIndex = (page * 128) + 90; // Start at Column 90
  uint8_t* buf = display.getBuffer();       // Direct access to Adafruit RAM array

  // 3. Draw and erase simultaneously across the 35-pixel block
  for (uint8_t i = 0; i < 35; i++) {
    if (i == 0 || i == 34) {
      buf[bufferIndex + i] = 0x7F; // Draw solid box borders (01111111)
    } else if (i <= filledWidth) {
      buf[bufferIndex + i] = 0x7F; // Fill inside of the active slider
    } else {
      buf[bufferIndex + i] = 0x41; // Clear old fills, leaving only top/bottom frame lines (01000001)
    }
  }
}

// =========================================================================
// 8. OPTIMIZED PARTIAL HORIZONTAL DISPLAY HARDWARE STREAM PIPELINE
// =========================================================================
void displayPartialHorizontalWindow(uint8_t x_start, uint8_t y_row_start, uint8_t width, uint8_t row_count) {
    uint8_t *buffer = display.getBuffer();
    //if (!buffer) return; //Probably not needed

    // Boundary constraints to protect hardware indexing
    if (x_start + width > 128) width = 128 - x_start;
    if (y_row_start + row_count > 8) row_count = 8 - y_row_start;
    uint8_t x_end = x_start + width - 1;
    uint8_t y_row_end = y_row_start + row_count - 1;

    // 1. Isolate the target hardware coordinate grid registers
    Wire.beginTransmission(0x3C);
    Wire.write(0x00); // Command execution mode indicator
    Wire.write(0x21); // Command: Set Column Address Window Range
    Wire.write(x_start);
    Wire.write(x_end);
    Wire.write(0x22); // Command: Set Page/Row Address Window Range
    Wire.write(y_row_start);
    Wire.write(y_row_end);
    Wire.endTransmission();

    // 2. Stream out the data bytes sequentially matching Horizontal mode wrapping rules
    // Because the display is in Horizontal mode, it increments X across rows natively!
    for (uint8_t r = y_row_start; r <= y_row_end; r++) {
        uint16_t row_offset = r * 128;
        uint8_t bytesSent = 0;
        
        for (uint8_t x = x_start; x <= x_end; x++) {
            if (bytesSent == 0) {
                Wire.beginTransmission(0x3C);
                Wire.write(0x40); // Pixel stream data indicator flag
            }
            
            Wire.write(buffer[row_offset + x]);
            bytesSent++;
            
            // I2C protocol constraint: Split transmission into 16-byte blocks
            if (bytesSent >= 16) {
                Wire.endTransmission();
                bytesSent = 0;
            }
        }
        if (bytesSent > 0) {
            Wire.endTransmission();
        }
    }
}

// =========================================================================
// 8a. DISPLAY HARDWARE COMMAND (ON/OFF) Setting
// =========================================================================

void screenSet (uint8_t command){
        Wire.beginTransmission(0x3C);
    Wire.write(0x00); // Command execution mode indicator
   // Wire.write(0x81); // Command: Set Contrast Control register - (Not working)
    Wire.write(command); // Value: 0xAF (On) to 0xAE (Off/Powersave)
    Wire.endTransmission();

}
// =========================================================================
// *** DYNAMIC ENCODER ACCELERATION CALCULATOR ***
// =========================================================================
long calculateAcceleratedDelta(long rawDelta) {
    static unsigned long lastEncoderMoveTime = 0;
    unsigned long now = millis();
    unsigned long timeElapsed = now - lastEncoderMoveTime;
    lastEncoderMoveTime = now;

    // Default step behavior for slow turns
    long acceleratedDelta = rawDelta;

    // If the knob is turned quickly (less than 50ms between clicks)
    if (timeElapsed < 50) {
        if (timeElapsed < 15) {
            acceleratedDelta = rawDelta * 15; // Ultra-fast spin: massive jump
        } else if (timeElapsed < 30) {
            acceleratedDelta = rawDelta * 6;  // Medium-fast spin: medium jump
        } else {
            acceleratedDelta = rawDelta * 2;  // Casual fast spin: double jump
        }
    }

    return acceleratedDelta;
}

// =========================================================================
// *** FULL SYSTEM RESET ***
// =========================================================================
void resetSystem() {
   // 1. Hard-reset the SCB-55 daughterboard
    digitalWrite(SCB55_RESET_PIN, LOW);
    //delay(50); // Hold down the reset line for 50ms
    //digitalWrite(SCB55_RESET_PIN, HIGH);
    
    // 2. Reset local software variables (e.g., reset your encoder counts to 0)
    // encoder1Count = 0;
    // encoder2Count = 0;
    
    tinyDelay(100); // Allow SCB-55 power rails to stabilize
    
    // 3. Force the Teensy 2.0 microcontroller to restart its firmware from scratch
    // Using the internal AVR watchdog timer to trigger a clean software reboot
    cli(); 
    UDCON = 1; USBCON = (1<<FRZCLK); // Shut down USB cleanly
    tinyDelay(20);
    // Force a jump to the chip's absolute starting memory address
    asm volatile("jmp 0"); 
}

// =========================================================================
// 9. SETUP INITIALIZATION
// =========================================================================
void setup() {
    Serial1.begin(31250); 
    //Serial.begin(115200); //Debug
    //Serial.println(F("Debug!")); 
    pinMode(ENC_SW_PIN, INPUT_PULLUP);
    pinMode(POT_PIN, INPUT);
    pinMode(ENC_A_PIN, INPUT_PULLUP);
    pinMode(ENC_B_PIN, INPUT_PULLUP);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    pinMode(SCB55_RESET_PIN, OUTPUT); 
    digitalWrite(SCB55_RESET_PIN, LOW); 
    tinyDelay(50); // Hold down the SBC-55 reset line for 50ms
    digitalWrite(SCB55_RESET_PIN, HIGH);  
    
    //display.cp437(true); //Fix higher fonts from being shifted - 6 bytes for missing font 0xB2

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        for(;;); //No screen buffer, check RAM free > 1024
    }
    
    //int initialPotRead = analogRead(POT_PIN);  //Not needed, removed to save space
    //currentVolume = map(initialPotRead, 0, 1023, 0, 127);
    
    inboundReset(); // Sets Vol & Rev array defaults and below code
    //commitPatchChange(0); 

    // --- Safe System Startup Clearing Sequence ---
    //display.clearDisplay(); // Erases all raw startup garbage from canvas cache
    //display.display();      // Forces a full, clear black frame rewrite to the panel registers
    
    // Draw the foundational frame lines and static title labels
    //drawStaticUIFrame(); 
}

// =========================================================================
// 10. LOOP EXECUTION
// =========================================================================
void loop() {
    if (!(PINC & (1 << 7))) { tinyDelay(200); if (!(PINC & (1 << 7))) { // Direct read of Pin 10 (PC7)
    resetSystem(); }} // Delay for accidental button press
//  --- 1a. PC Host USB MIDI Input Handler ---
    while (usbMIDI.read()) { 
        uint8_t type    = usbMIDI.getType();
        // OPTIMIZATION: Drop Real-Time Clock flooding (synth doesn't need to sync tempo)
        if (type == 0xF8 || type == 0xFE) continue; //Go to loop start to process next packet immediately
        uint8_t channel = usbMIDI.getChannel();
        uint8_t d1      = usbMIDI.getData1();
        uint8_t d2      = usbMIDI.getData2();
        activity = true;

        if (type < 0xF0 ) {
            Serial1.write((type & 0xF0) | ((channel - 1) & 0x0F));
            Serial1.write(d1);
            if ((type & 0xF0) != 0xC0 && (type & 0xF0) != 0xD0) { 
                Serial1.write(d2);
            }
            // If it's Pitch Bend E0, Note On 90, Note Off 80, Aftertouch A0, or Ch Pressure D0, skip to next byte
            uint8_t statusNibble = type & 0xF0;
            if (statusNibble == 0xE0 || statusNibble == 0x90 || statusNibble == 0x80 || statusNibble == 0xA0 || statusNibble == 0xD0) {
                continue; }
         // 2. STATE INTERCEPT PATH (Only runs for specific UI parameters)
         if (type == usbMIDI.ControlChange) {
            
            switch (d1) { // Check CC parameters cleanly using an optimized switch & update if changed
                case 0:
                    chanCurrentBank[channel - 1] = d2; updateDisplayFlag = true; continue;
                case 7:
                    if (chanVolume[channel - 1] != d2) {chanVolume[channel - 1] = d2; updateDisplayFlag = true;
                } continue;
                case 91:
                    if (chanReverb[channel - 1] != d2) {chanReverb[channel - 1] = d2; updateDisplayFlag = true;
                } continue;
                case 93:
                    if (chanChorus[channel - 1] != d2) {chanChorus[channel - 1] = d2; updateDisplayFlag = true;
                } continue;
            }
         } 
         else if (type == usbMIDI.ProgramChange) {
            synchronizeUIWithIncomingPatch(channel, chanCurrentBank[channel - 1], d1);
         continue; }
        }    //  --- SysEX Display Packet Handling from USB & fall through for unprocessed MIDI bytes
         
    else if (type == usbMIDI.SystemExclusive) {
            const byte *sysExData = usbMIDI.getSysExArray();
            unsigned int sysExLen = d1 + (d2 * 256); 
            if (sysExLen != 74) Serial1.write(sysExData, sysExLen); // No 74 byte 16x16 graphic sent to SCB-55
         // Length>=10  0x41=Roland
         //             ch=10-1F Model:0x16=MT32 0x42/0x45=SC-55 
         //             0x12=command 0x20/0x18=MT/SC address (width=20chr)
        if (sysExLen <= 9) {
            // Put short SysEx checks here
                        if (sysExData[1] == 0x7E && sysExData[2] == 0x7F && sysExData[3] == 0x09 && sysExData[4] == 0x01) {
                        inboundReset(); return; }//gmReset received! 

            return;
        }

        // Inline validation: Check Vendor, Command, and Channel ID bounds in one step
        if (sysExData[1] != 0x41 || sysExData[4] != 0x12 || (uint8_t)(sysExData[2] - 0x10) > 0x0F) return;

        // Cache frequent memory lookups directly into CPU working registers
        uint8_t mID  = sysExData[3]; // Model MT-32 (0x16) / SC-55 / GM (0x42/0x45)
        uint8_t aHi  = sysExData[5]; // Address High
        uint8_t aMid = sysExData[6]; // Address Mid

        switch (mID) {
            case 0x16:
                if (aHi == 0x20) sysExTextLine(sysExData, sysExLen); //Move sysEx Text to screen variable
                break;

            case 0x42:
            // Drum Channel Bitmask Mapping Block
            // Validates: aHi (sysExData[5]) == 0x40, sysExData[7] == 0x15, and aMid (sysExData[6]) high nibble == 0x10
        if (aHi == 0x40 && sysExData[7] == 0x15 && (aMid & 0xF0) == 0x10) {
            
            // Dynamic Roland to standard MIDI channel conversion
            uint8_t ch = (aMid & 0x0F);
            ch = (ch == 0) ? 10 : (ch > 9) ? ch + 1 : ch;

            // Update 16-bit drum channel tracking map using the payload parameter value (sysExData[8])
            channelMap = (sysExData[8] > 0) ? (channelMap | (1U << (ch - 1))) : (channelMap & ~(1U << (ch - 1)));
            break; 
        }
            case 0x45: // Fall-through groups shared logic for fast execution
                if (aHi == 0x10) {
                    if (aMid == 0x00) {
                        sysExTextLine(sysExData, sysExLen); //Move sysEx Text to screen variable
                    } else if (sysExLen == 74 && aMid == 0x01 && sysExData[7] == 0x00) { // USB_MIDI_SYSEX_MAX 79 in usb_api.h
                        // Extract raw data chunk (pointer shifts past the 8 baseline header bytes)
                        injectPayloadToDisplayBuffer(sysExData + 8);
                    }
                } else if (mID == 0x42 && aHi == 0x40 && aMid == 0x00 && sysExData[7] == 0x7F && sysExData[8] == 0x00) {
                    inboundReset(); return; //gsReset received!
                }
                break;
        }

        } //end of SysEx
        Serial1.write(type); //If we missed anything, send it.
    } //end of While usbMIDI

 //  --- 1b. Hardware DIN Serial MIDI Pass-Through Merger ---
    static uint8_t serialMidiStatus = 0;
    static uint8_t serialMidiCount = 0;
    static uint8_t serialMidiD1 = 0;
    static uint8_t serialRxChan = 0;
    static uint8_t serialEat = 0; //for tracking serial bytes to skip & activity monitor
    bool parserIsIdle = true;

    while (Serial1.available() > 0) {
        uint8_t incomingByte = Serial1.read();
        //usbMIDI.send_now(); //To prevent buffer stall from disconnected USB MIDI

        // Drop Clock F8 FE, Start FA, Continue FB, Stop FC, Reset FF & the two undefined Real Time Bytes
        if (incomingByte >= 0xF8) { continue; } 
                
        if (incomingByte == MIDI_SYSEX_START || isRecordingSysEx) {
            processSysEx(incomingByte); continue; } 

        Serial1.write(incomingByte); //If no SysEx to process

            if (serialEat != 0) {serialEat--; continue;}
            switch (incomingByte & 0xF0) {
                case 0x80: serialEat = 2; continue; //Restart loop after (untracked) Note Off sent (pass & eat next two bytes)
                case 0x90: serialEat = 2; continue; //Restart loop after (untracked) Note On sent (pass & eat next two bytes)
                case 0xA0: serialEat = 2; continue; //Restart loop after (untracked) Aftertouch sent (pass & eat next two bytes)
                case 0xD0: serialEat = 1; continue; //Restart loop after (untracked) Ch Pressure sent (pass & eat next byte)
                case 0xE0: serialEat = 2; continue; //Restart loop after (untracked) Pitch Bend sent (pass & eat next two bytes)
            } 

    // 1. Process all available bytes in the hardware serial buffer
        parserIsIdle = false; // Parser is actively processing a stream

        if (incomingByte >= 0x80) {
            if (incomingByte < 0xF0) {
                serialMidiStatus = incomingByte & 0xF0;
                serialRxChan = incomingByte & 0x0F;
                
                // Array bounds protection
                if (serialRxChan < 16) {
                    serialMidiCount = 1;
                } else {
                    serialMidiCount = 0;
                }
            } else {
                serialMidiCount = 0;
            }
        } 
        else if (serialMidiCount > 0) {
            if (serialMidiCount == 1) {
                serialMidiD1 = incomingByte;
                if (serialMidiStatus == 0xC0) {
                    synchronizeUIWithIncomingPatch(serialRxChan + 1, chanCurrentBank[serialRxChan], serialMidiD1);
                    serialMidiCount = 1; // Support Running Status
                } else {
                    serialMidiCount = 2;
                }
            } 
            else if (serialMidiCount == 2) {
                if (serialMidiStatus == 0xB0) {
                    if (serialMidiD1 == 0) {
                        chanCurrentBank[serialRxChan] = incomingByte;
                        updateDisplayFlag = true;
                    } else if (serialMidiD1 == 7) {
                        if (chanVolume[serialRxChan] != incomingByte) {
                            chanVolume[serialRxChan] = incomingByte; 
                            updateDisplayFlag = true; 
                        }
                    } else if (serialMidiD1 == 91) {
                        if (chanReverb[serialRxChan] != incomingByte) {
                            chanReverb[serialRxChan] = incomingByte; 
                            updateDisplayFlag = true; 
                        }
                    } else if (serialMidiD1 == 93) {
                        if (chanChorus[serialRxChan] != incomingByte) {
                            chanChorus[serialRxChan] = incomingByte; 
                            updateDisplayFlag = true; 
                        }
                    }
                }
                serialMidiCount = 1; // Reset for Running Status
            }
        }
    }

    // When the while loop finishes, the hardware buffer is empty
    parserIsIdle = true; 

 //  --- 2. Volume Potentiometer Processing --- (Throttled to every 30ms)
    if ((uint16_t)millis() - lastAnalogPoll >= 30) { //was 15
        lastAnalogPoll = (uint16_t)millis();
        int rawAnalog = analogRead(POT_PIN);
        if (abs(rawAnalog - lastAnalogValue) > 5) { 
            //uint8_t calculatedVol = map(rawAnalog, 0, 1023, 0, 127);
            uint8_t calculatedVol = rawAnalog >> 3; //Saves code space vs map()
            if ((calculatedVol != currentVolume) && parserIsIdle) {
                currentVolume = calculatedVol;
                //sendMidiControlChange(currentChannel, 7, currentVolume); //Changed to Master Volume
                //chanVolume[currentChannel - 1] = currentVolume; 
                sendGSSysEx(0x40, 0x00, 0x04, currentVolume); //Can be overridden by Global Menu
                volumeAnalogDisplay = true;
                updateDisplayFlag = true;
            }
            lastAnalogValue = rawAnalog;
        }  
    }

 //  --- 3. Hardware Rotary Quadrature Tracking (Accelerated) ---
    long currentTicks = myEnc.read();
    long tickDifference = currentTicks - lastEncoderTicks;
    
    // Check if the hardware physically moved past a mechanical detent (2 raw ticks)
    if (tickDifference >= 2 || tickDifference <= -2) {
        long rawDelta = tickDifference / 2;
        lastEncoderTicks = currentTicks; // Latch new base instantly

        // Pass the raw change into our calculator to compute velocity jumps
        long delta = calculateAcceleratedDelta(rawDelta);

    //long delta = tickDifference >> 1; //Same as devide by 2 but quicker

 //  --- 3a. Menu Navigation
            switch (currentMode) {
                case SET_CHANNEL: { 
                    int ch = (int)currentChannel - 1 + delta;
                    ch = ch % 16;
                    if (ch < 0) ch += 16;
                    currentChannel = (uint8_t)(ch + 1);
                    
                    cursorIndex = chanActivePatch[currentChannel - 1];
                    activeIndex = chanActivePatch[currentChannel - 1];
                    break;
                }

                case BROWSE_PATCH: { 
                    int maxPatches = (isDrumPatch(currentChannel)) ? 10 : 354;
                    cursorIndex = (cursorIndex + delta * parserIsIdle) % maxPatches;
                    if (cursorIndex < 0) cursorIndex += maxPatches;
                    commitPatchChange(cursorIndex); 
                    break;
                }

                case MAIN_SYSTEM_MENU: {
                    systemMenuSelection = (systemMenuSelection + (int8_t)delta) % 4; 
                    if (systemMenuSelection < 0) systemMenuSelection += 4;
                    break;
                }

                case SYSEX_RESETS_MENU: {
                    sysExSelection = (sysExSelection + (int8_t)delta) % 4; 
                    if (sysExSelection < 0) sysExSelection += 4;
                    break;
                }

                case FX_SETTINGS_MENU: {
                    if (!isEditingParam) {
                        fxSelectedParam = (fxSelectedParam + (int8_t)delta) % 4;
                        if (fxSelectedParam < 0) fxSelectedParam += 4;
                    } else if (parserIsIdle) {
                        if (fxSelectedParam == 0) {
                            masterVolumeState = constrain(masterVolumeState + delta, 0, 127);
                            sendGSSysEx(0x40, 0x00, 0x04, masterVolumeState);
                        } else if (fxSelectedParam == 1) {
                            masterReverbState = constrain(masterReverbState + delta, 0, 127);
                            sendGSSysEx(0x40, 0x01, 0x33, masterReverbState);
                        } else if (fxSelectedParam == 2) {
                            masterChorusState = constrain(masterChorusState + delta, 0, 127);
                            sendGSSysEx(0x40, 0x01, 0x3A, masterChorusState);
                        }
                    }
                    break;
                }

                case CHAN_FX_MENU: {
                    if (!isEditingParam) {
                        chanFxSelectedParam = (chanFxSelectedParam + (int8_t)delta) % 5;
                        if (chanFxSelectedParam < 0) chanFxSelectedParam += 5;
                    } else if (parserIsIdle){
                        if (chanFxSelectedParam == 0) {
                            int ch = (int)chanFxChannel - 1 + delta;
                            ch = ch % 16; if (ch < 0) ch += 16;
                            chanFxChannel = ch + 1;
                        } else if (chanFxSelectedParam == 1) {
                            chanVolume[chanFxChannel - 1] = constrain(chanVolume[chanFxChannel - 1] + delta, 0, 127);
                            sendMidiControlChange(chanFxChannel, 7, chanVolume[chanFxChannel - 1]);
                        } else if (chanFxSelectedParam == 2) {
                            chanReverb[chanFxChannel - 1] = constrain(chanReverb[chanFxChannel - 1] + delta, 0, 127);
                            sendMidiControlChange(chanFxChannel, 91, chanReverb[chanFxChannel - 1]);
                        } else if (chanFxSelectedParam == 3) {
                            chanChorus[chanFxChannel - 1] = constrain(chanChorus[chanFxChannel - 1] + delta, 0, 127);
                            sendMidiControlChange(chanFxChannel, 93, chanChorus[chanFxChannel - 1]);
                        }
                    }
                    break;
                }
            }
        updateDisplayFlag = true;
        
            // Increment history ONLY by the whole detent steps we actually processed.
            // This leaves any leftover fractional ticks (half-steps) intact for the next loop.
            //lastEncoderTicks += (delta * 2); slow version replaced above by (delta <<1)
    }

 //  --- 4. Button Timing State Machine ---
    uint8_t buttonReading = digitalRead(ENC_SW_PIN);

    if (buttonReading != lastButtonReading) {
        lastDebounceTime = (uint16_t)millis();
    }
    lastButtonReading = buttonReading;

    if ((uint16_t)(millis() - lastDebounceTime) > debounceDelay) {
        if (buttonReading != stableButtonState) {
            stableButtonState = buttonReading;
            
            if (stableButtonState == LOW && previousStableButtonState == HIGH) {
                buttonPressTime = (uint16_t)millis();
                if (clickCount == 1 && ((uint16_t)(millis() - lastClickTime) < doubleClickThreshold)) {
                    triggerMidiPanic();
                    clickCount = 0;
                    ignoreNextRelease = true; 
                }
            }
            
            if (stableButtonState == HIGH && previousStableButtonState == LOW) {
                if (ignoreNextRelease) {
                    ignoreNextRelease = false; 
                } else {
                    uint16_t holdDuration = (uint16_t)(millis() - buttonPressTime);
                    if (holdDuration >= longPressThreshold) {
                        if (currentMode == MAIN_SYSTEM_MENU || currentMode == SYSEX_RESETS_MENU || currentMode == FX_SETTINGS_MENU || currentMode == CHAN_FX_MENU) {
                            currentMode = SET_CHANNEL; //drawStaticUIFrame();         // Force-draw the static line frames once
                            isEditingParam = false;
                        } else {
                            currentMode = MAIN_SYSTEM_MENU; //drawStaticUIFrame();    // Force-draw the static line frames once
                            systemMenuSelection = 0;
                            inChannelMenu = true; //flag to save draws on menu
                        }
                        drawStaticUIFrame();  // Force-draw the static line frames once
                        clickCount = 0; 
                        updateDisplayFlag = true; 
                    } else {
                        clickCount = 1;
                        lastClickTime = (uint16_t)millis();
                    }
                }
            }
            previousStableButtonState = stableButtonState;
        }
    }

 //  --- 4a. Menu Navigation
    if (clickCount == 1 && ((uint16_t)millis() - lastClickTime >= doubleClickThreshold)) {
        if (currentMode == MAIN_SYSTEM_MENU) {
            if (systemMenuSelection == 0) {
                currentMode = SYSEX_RESETS_MENU;// drawStaticUIFrame();  // Force-draw the static line frames once
                sysExSelection = 0;
                inChannelMenu = true; //flag to save draws on menu
            } else if (systemMenuSelection == 1) {
                currentMode = FX_SETTINGS_MENU; // drawStaticUIFrame();  // Force-draw the static line frames once
                fxSelectedParam = 0;
                isEditingParam = false;
                inChannelMenu = true; //flag to save draws on menu
            } else if (systemMenuSelection == 2) {
                currentMode = CHAN_FX_MENU;     // drawStaticUIFrame();  // Force-draw the static line frames once
                chanFxSelectedParam = 0;
                isEditingParam = false;
            } else {
                currentMode = SET_CHANNEL;      // drawStaticUIFrame();  // Force-draw the static line frames once
            }
            drawStaticUIFrame();  // Force-draw the static line frames once
        } 
        else if (currentMode == SYSEX_RESETS_MENU) {
            if (sysExSelection == 3) {
                currentMode = MAIN_SYSTEM_MENU;  drawStaticUIFrame();  // Force-draw the static line frames once
                inChannelMenu = true; //flag to save draws on menu
            } else {
                executeSysExTransfer(sysExSelection);
                currentMode = SET_CHANNEL;       drawStaticUIFrame();  // Force-draw the static line frames once
            }
        } 
        else if (currentMode == FX_SETTINGS_MENU) {
            if (fxSelectedParam == 3) {
                currentMode = MAIN_SYSTEM_MENU;  drawStaticUIFrame();  // Force-draw the static line frames once
                inChannelMenu = true; //flag to save draws on menu
            } else {
                isEditingParam = !isEditingParam; 
            }
        } 
        else if (currentMode == CHAN_FX_MENU) {
            if (chanFxSelectedParam == 4) {
                currentMode = MAIN_SYSTEM_MENU;  drawStaticUIFrame();  // Force-draw the static line frames once
                inChannelMenu = true; //flag to save draws on menu
            } else {
                isEditingParam = !isEditingParam;
            }
        }
        else if (currentMode == SET_CHANNEL) {
            currentMode = BROWSE_PATCH;          drawStaticUIFrame();  // Force-draw the static line frames once
            inChannelMenu = true; //flag to save draws on Browse menu
        } 
        else if (currentMode == BROWSE_PATCH) {
            currentMode = SET_CHANNEL;           drawStaticUIFrame();  // Force-draw the static line frames once
        }
        clickCount = 0;
        updateDisplayFlag = true;
    }

 //  --- 5. Deferred UI Rendering Refresh ---
    if (updateDisplayFlag && ((uint16_t)millis() - lastScreenUpdateTime >= refreshRateMaxMs)) {
        activity = true;
        renderInterfaceUI();
        updateDisplayFlag = false;
        lastScreenUpdateTime = (uint16_t)millis();
        screenSaver = lastScreenUpdateTime;
    }
 //  --- 6a. Non-blocking Inactivity Screen Blanker
    if (serialEat || activity) { screenSaver = (uint16_t)millis(); activity = false;
        if (!screenOn) screenSet(0xAF); //Turn on OLED
    } else {
             if ((uint16_t)millis() - screenSaver >= 40000) screenSet(0xAE); screenOn = false; 
            }

 //  --- 6b. Non-blocking Screen Message Monitor: Check if the text / graphics frame buffer should time out
    if (isDisplayActive && (millis() - lastMidiMessageTime >= messageTimeoutMs)) {
    clearTextDisplayWindow();
    }

}