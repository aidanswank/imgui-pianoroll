#pragma once

#include "midifile/MidiFile.h"
#include "midifile/Options.h"
#include <string>

struct midiQ
{
    float sampleIndex;
    smf::MidiEvent event;
};

//visual representation NOT actual play data
struct Note
{
    int key;
    int start;
    int duration;
    smf::MidiEvent *endNote;
    smf::MidiEvent *startNote;
};

struct MidiTrack
{
    std::string name;
   int minNote = 64;
   int maxNote = 64;
   int tpq;
   std::vector<Note> notes;
//    std::vector<smf::MidiEvent> events;
   smf::MidiEventList *eventlist;
};

struct MidiFileWrapper
{
    smf::MidiFile midifile;
    smf::Options options;

    std::string filename;
    int trackCount;
    int tpq;
    float mspt;

    // nlohmann::json *project;

    int sample_rate;
    float bpm;

    // load midi wrapped midifile object
    int init(std::string p_filename)
    {
        filename = p_filename;
        std::cout << "loaded " << filename << std::endl;

        // project = p_project;

        // sample_rate = (int)project->at("sample_rate");
        // bpm = (float)project->at("bpm");

        int err = midifile.read(filename.c_str());
        midifile.doTimeAnalysis();
        midifile.linkNotePairs();

        trackCount = midifile.getTrackCount();
        tpq = midifile.getTicksPerQuarterNote();

        // mspt = getMSPerTick(bpm, tpq);

        std::cout << "TPQ: " << tpq << std::endl;
        std::cout << "TRACKS: " << trackCount << std::endl;
        // std::cout << "MSPT: " << mspt << std::endl;

        print("track count", midifile.getTrackCount());

        return err;
    }

    float getMSPerTick(int bpm, int tpq)
    {
        float ms = 60000 / (float)(bpm * tpq);
        return ms;
    }

    float samples2Ticks(float samps)
    {
        return (1000*samps)/(mspt*sample_rate); // t

        // return ((1000*samps)/mspt)/musicbox.project["sample_rate"; // t
    }

    std::vector<midiQ> getEventsBetweenBlocks(int start, int end)
    {
        // std::cout << "start " << (float)start/musicbox.project["sample_rate" << " end " << (float)end/musicbox.project["sample_rate" << '\n'; 
        // float tick = samples2Ticks(start);
        // std::cout << "start " << samples2Ticks(start) << " end " << samples2Ticks(end) << '\n'; 

        int tstart = samples2Ticks(start);
        int tend = samples2Ticks(end);
        // std::cout << "start " << tstart << " end " << tend << '\n'; 

        // std::vector<smf::MidiEvent> events;
        std::vector<midiQ> events;

        if(tstart!=tend)
        {
            for (int track = 0; track < trackCount; track++)
            {
                for (int event = 0; event < midifile[track].size(); event++)
                {
                    if(midifile[track][event].tick>=tstart&&midifile[track][event].tick<=tend){
                        midiQ q;
                        q.sampleIndex = (float)midifile[track][event].seconds*sample_rate;
                        q.event = midifile[track][event];
                        events.push_back(q);
                    }
                }
            }
        }

        // for(int i = 0; i < events.size(); i++)
        // {
        //     std::cout << events[i].sampleIndex << " " << (int)events[i].event[0] << " " << (int)events[i].event[1] << " " << (int)events[i].event[2] << std::endl;
        // }

        return events;

    }

    std::vector<MidiTrack> makeStructs()
    {
        int tracks = midifile.getTrackCount();
        std::vector<MidiTrack> trackVec;

        for (int track = 0; track < tracks; track++)
        {
            MidiTrack trackStruct;

            // std::cout << track << std::endl;
            // std::cout << "Tick\tSeconds\tDur\tMessage" << std::endl;

            for (int event = 0; event < midifile[track].size(); event++)
            {
                // std::cout << std::dec << midifile[track][event].tick;
                // std::cout << '\t' << std::dec << midifile[track][event].seconds;
                // std::cout << '\t';

                Note note;
                // trackStruct.events.push_back(midifile[track][event]);

                smf::MidiEventList mytrack = midifile[track];
                trackStruct.eventlist = &midifile[track];

                // hacky????
                trackStruct.tpq = midifile.getTicksPerQuarterNote();

                // note.eventIndex = event;

                if (midifile[track][event].isTrackName())
                { // check for track names
                    std::string content = midifile[track][event].getMetaContent();
                    if (!content.empty())
                    {
                    std::cout << content << std::endl;
                    trackStruct.name = content;
                    }
                }

                note.start = midifile[track][event].tick;
                // std::cout << '\t';

                if (midifile[track][event].isTempo())
                {
                    double mspqn = midifile[track][event].getTempoTPS(tpq);
                    std::cout << "mspqn: " << mspqn << std::endl;
                }

                if (midifile[track][event].isLinked())
                {
                    if (midifile[track][event].isNoteOn())
                    {
                    note.duration = midifile[track][event].getTickDuration();
                    note.key = midifile[track][event].getKeyNumber();
                    note.startNote = &midifile[track][event];
                    note.endNote = midifile[track][event].getLinkedEvent();
                    // print("note info",note.duration,note.key,note.startNote,note.endNote);
                    // end note
                    //  std::cout << midifile[track][event].getLinkedEvent() << std::endl;

                    trackStruct.notes.push_back(note);

                    // min max
                    trackStruct.minNote = std::min(trackStruct.minNote, note.key);
                    trackStruct.maxNote = std::max(trackStruct.maxNote, note.key);
                    }
                }
            }

            // print(trackStruct.notes.size());
            // filter dead tracks
            if(trackStruct.notes.size()!=0)
            {
                trackVec.push_back(trackStruct);
            }
        }

        return trackVec;

    }

    void printData()
    {
        std::cout << "count" << trackCount << std::endl;
        for (int track = 0; track < trackCount; track++)
        {
            // MidiTrack trackStruct;

            std::cout << "track: "<< track << std::endl;
            std::cout << "Tick\tSeconds\tDur\tMessage\tsamp" << std::endl;

            for (int event = 0; event < midifile[track].size(); event++)
            {
                // std::cout << std::dec << midifile[track][event].tick;
                // std::cout << '\t' << std::dec << midifile[track][event].seconds;

                std::cout << std::dec << midifile[track][event].tick;
                std::cout << '\t' << std::dec << midifile[track][event].seconds;

                // Note note;
                // trackStruct.events.push_back(midifile[track][event]);

                std::cout << '\t';
                if (midifile[track][event].isNoteOn()){
                    std::cout << '\t';
                    std::cout << midifile[track][event].getDurationInSeconds();
                }
                std::cout << '\t';
                
                std::cout << (int)midifile[track][event][0] << " " << 
                (int)midifile[track][event][1] << " " << (int)midifile[track][event][2];
                std::cout << '\t';
                std::cout << " mspt " << (float)midifile[track][event].seconds*sample_rate;
                // std::cout << " " << samples2Ticks(mspt*(float)midifile[track][event].tick*(sample_rate/1000));
                std::cout << std::endl;
                // smf::MidiEventList mytrack = midifile[track];
                // trackStruct.eventlist = &midifile[track];

                // note.eventIndex = event;

                if (midifile[track][event].isTrackName())
                {   // check for track names
                    std::string content = midifile[track][event].getMetaContent();
                    if (!content.empty())
                    {
                        std::cout << "content: " << content << std::endl;
                        // trackStruct.name = content;
                    }
                }

                // note.start = midifile[track][event].tick;
//                std::cout << '\t';

                // if (midifile[track][event].isTempo())
                // {
                //     double mspqn = midifile[track][event].getTempoTPS(tpq);
                //     // std::cout << "mspqn: " << mspqn << std::endl;
                // }

                if (midifile[track][event].isLinked())
                {
                    if (midifile[track][event].isNoteOn())
                    {
                        //         note.duration = midifile[track][event].getTickDuration();
                        //         note.key = midifile[track][event][1];
                        //         note.startNote = &midifile[track][event];
                        //         note.endNote = midifile[track][event].getLinkedEvent();
                        //         // end note
                        //         //  std::cout << midifile[track][event].getLinkedEvent() << std::endl;
                        //std::cout << "is on" << std::endl;
                        //         trackStruct.notes.push_back(note);

                        //         // min max
                        //         trackStruct.minNote = std::min(trackStruct.minNote, note.key);
                        //         trackStruct.maxNote = std::max(trackStruct.maxNote, note.key);
                    }
                }
            }

            // trackVec.push_back(trackStruct);
        }
    }

};
