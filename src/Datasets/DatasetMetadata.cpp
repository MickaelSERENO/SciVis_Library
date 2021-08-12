#include "Datasets/DatasetMetadata.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <json/value.h>
#include <json/reader.h>
#include "sciVisUtils.h"

namespace sereno
{
    DatasetMetadata::DatasetMetadata(const std::string& filePath)
    {
        //Read the file (uint8_t*)
        std::ifstream inFile;
        inFile.open(filePath);
        std::stringstream strStream;
        strStream << inFile.rdbuf(); //read the file
        std::string str = strStream.str();
        const char* jsonFile = str.c_str(); //str holds the content of the file

        //Parse the json text
        Json::CharReaderBuilder jBuilder;
        std::unique_ptr<Json::CharReader> const jReader(jBuilder.newCharReader());
        Json::Value jRoot;
        if(!jReader->parse(jsonFile, jsonFile+str.size(), &jRoot, NULL))
        {
            WARNING << "Could not parse the configuration json text\n";
            return;
        }

        //Read timesteps metadata
        auto& jTimesteps = jRoot["perTimesteps"];
        if(jTimesteps && jTimesteps.isArray())
        {
            for(auto& timestep : jTimesteps)
            {
                PerTimestepMetadata perTimestepMD;

                auto& date = timestep["date"];
                if(date && date.isString())
                    perTimestepMD.date = date.asString();
                perTimestepMetadata.push_back(perTimestepMD);
            }
        }

        //Read the coastline filepath
        auto& jCoastline = jRoot["coastline"];
        if(jCoastline && jCoastline.isString())
                coastlinePath = jCoastline.asString();
    }
}
