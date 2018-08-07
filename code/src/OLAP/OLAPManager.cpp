//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <boost/algorithm/string.hpp>

#include "../Common/stdafx.h"
#include "../OLAP/OLAPManager.h"
#include <string>

OLAPManager * OLAPManager::OLAP = NULL;

OLAPManager::OLAPManager()
{
    numVerticesMaterialized = -1;
}

OLAPManager::~OLAPManager()
{
    //dtor
}

OLAPManager* OLAPManager::getInstance(void)
{
	if(OLAP==NULL)
	{
		OLAP = new OLAPManager();
	}
	return OLAP;
}


void OLAPManager::mapQueryIDCubifyOp(int queryID, boost::shared_ptr<CubifyOperator> cubifyOp)
{
    this->queryIDCubifyOpMap.insert(std::make_pair(queryID, cubifyOp));
}

boost::shared_ptr<CubifyOperator> OLAPManager::getCubifyOpByQueryID(int queryID)
{
    std::map<int, boost::shared_ptr<CubifyOperator> >::iterator queryIDCubifyOpMapIt;
	queryIDCubifyOpMapIt = queryIDCubifyOpMap.find(queryID);

	if(queryIDCubifyOpMapIt != queryIDCubifyOpMap.end())
        return queryIDCubifyOpMapIt->second;
    else
    {
        std::cout << "OLAPManager: Cubify Operator with given query ID not found!" << std::endl;
        exit(0);
    }
}

void OLAPManager::parseCSVString(std::string csvStr, std::vector<std::string>& strVector)
{
    std::size_t pos1 = 0;
    std::size_t pos2;
    std::string valueStr;

    while(csvStr.find(',', pos1+1) != std::string::npos)
    {
        pos2 = csvStr.find(',',pos1+1);
        valueStr = csvStr.substr(pos1+1, pos2 - (pos1+1));
        boost::trim(valueStr);
        strVector.push_back(valueStr);
        pos1 = pos2;
    }

    valueStr = csvStr.substr(pos1+1);
    boost::trim(valueStr);
    strVector.push_back(valueStr);
}

void OLAPManager::readCubifyConfiguration(std::string configFile, int queryID)
{
    //std::string cubifyConfigureFilePath = CUBIFY_CONFIGURE_FILE_PATH;
	//std::ifstream fin(cubifyConfigureFilePath.c_str());
	std::ifstream fin(configFile.c_str());

	std::string str;

	while(getline(fin,str))
	{
		// ignoring commented lines
		if(str.find('#') != std::string::npos)
            continue;

        // Parsing dimensions
        if(str.find("Dimensions") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                parseCSVString(str.substr(pos+1), this->dimensions);
                //for(std::vector<std::string>::iterator it = dimensions.begin(); it != dimensions.end(); it++)
                    //std::cout << *it << std::endl;
            }

            // Insert or replace
            cubifyConfigVectorMap[queryID]["Dimensions"] = this->dimensions;
		}

		// Parsing dimension names
        if(str.find("DimensionNames") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                parseCSVString(str.substr(pos+1), this->dimensionNames);
            }

            //std::cout << "dimensionNames " << dimensionNames.size() << std::endl;
            // Insert or replace
            cubifyConfigVectorMap[queryID]["DimensionNames"] = this->dimensionNames;
		}

		if(str.find("DimensionSizes") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                parseCSVString(str.substr(pos+1), this->dimensionSizes);
            }

            //std::cout << "dimensionNames " << dimensionNames.size() << std::endl;
            // Insert or replace
            cubifyConfigVectorMap[queryID]["DimensionSizes"] = this->dimensionSizes;
		}

		if(str.find("RefFrequencies") != std::string::npos)
        {
            std::string RFMode = cubifyConfigMap[queryID]["RFMode"];

            if(RFMode == "Custom") // If custom, use the given frequency in cubify configuration file
            {
                std::size_t pos = str.find('=');
                // if "=" found
                if(pos != std::string::npos)
                {
                    parseCSVString(str.substr(pos+1), refFrequencies);
                }
            }
            else
            {
                assignFrequency(queryID, RFMode);
            }

            // Insert or replace
            cubifyConfigVectorMap[queryID]["RefFrequencies"] = this->refFrequencies;
        }

		if(str.find("TimeGrain") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                std::string timeGranularity = str.substr(pos+1);
                boost::trim(timeGranularity);
                cubifyConfigMap[queryID]["TimeGrain"] = timeGranularity;
                //for(std::vector<std::string>::iterator it = dimensions.begin(); it != dimensions.end(); it++)
                    //std::cout << *it << std::endl;
            }
		}

		if(str.find("IoA") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                std::string IoA = str.substr(pos+1);
                boost::trim(IoA);
                cubifyConfigMap[queryID]["IoA"] = IoA;
            }
		}

        // Parsing materialized vertices
		if(str.find("MVertices") != std::string::npos)
        {
            std::size_t pos1 = str.find('=');
            std::size_t pos2;
            std::vector<std::string> tmpStrVector;

            // if "=" found
            if(pos1 != std::string::npos)
            {
                while((pos2 = str.find('|', pos1+1)) != std::string::npos)
                {
                    tmpStrVector.clear();
                    parseCSVString(str.substr(pos1+1, pos2 - (pos1+1)), tmpStrVector);
                    mVertices.push_back(tmpStrVector);
                    pos1 = pos2;
                }

                tmpStrVector.clear();
                parseCSVString(str.substr(pos1+1), tmpStrVector);
                mVertices.push_back(tmpStrVector);

                /*
                for(std::vector< std::vector<std::string> >::iterator it = mVertices.begin(); it != mVertices.end(); it++)
                {
                    for(std::vector<std::string>::iterator itt = it->begin(); itt != it->end(); itt++)
                            std::cout << *itt << std::endl;

                    std::cout << "|" << std::endl;
                }
                */

            }

            cubifyConfigNestedVectorMap[queryID]["MVertices"] = this->mVertices;
        }

        // Parsing LatticeOutputNodes
		if(str.find("LatticeOutputVertices") != std::string::npos)
        {
            std::size_t pos = str.find("none");
            // if "none" found
            if(pos != std::string::npos)
            {
                //std::cout << "none found" << std::endl;
                continue;
            }

            std::size_t pos1 = str.find('=');
            std::size_t pos2;
            std::vector<std::string> tmpStrVector;

            // if "=" found
            if(pos1 != std::string::npos)
            {
                while((pos2 = str.find('|', pos1+1)) != std::string::npos)
                {
                    tmpStrVector.clear();
                    parseCSVString(str.substr(pos1+1, pos2 - (pos1+1)), tmpStrVector);
                    latticeOutputVertices.push_back(tmpStrVector);
                    pos1 = pos2;
                }

                tmpStrVector.clear();
                parseCSVString(str.substr(pos1+1), tmpStrVector);
                latticeOutputVertices.push_back(tmpStrVector);

            }

            cubifyConfigNestedVectorMap[queryID]["LatticeOutputVertices"] = this->latticeOutputVertices;
        }

        if(str.find("MMode") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                std::string materializationMode = str.substr(pos+1);
                boost::trim(materializationMode);
                cubifyConfigMap[queryID]["MMode"] = materializationMode;
                //std::cout << "cubifyConfigMap[queryID][MMode]: " << cubifyConfigMap[0]["MMode"] << std::endl;
            }
		}

		if(str.find("RFMode") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                std::string RFMode = str.substr(pos+1);
                boost::trim(RFMode);
                cubifyConfigMap[queryID]["RFMode"] = RFMode;
                //std::cout << "cubifyConfigMap[queryID][MMode]: " << cubifyConfigMap[0]["MMode"] << std::endl;
            }
		}

		if(str.find("OptMethod") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                std::string optimizationMethod = str.substr(pos+1);
                boost::trim(optimizationMethod);
                cubifyConfigMap[queryID]["OptMethod"] = optimizationMethod;
            }
		}

		if(str.find("MVerticesNum") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                std::string materializedVerticesNum = str.substr(pos+1);
                boost::trim(materializedVerticesNum);
                cubifyConfigMap[queryID]["MVerticesNum"] = materializedVerticesNum;
                //std::cout << "cubifyConfigMap[queryID][MVerticesNum]: " << cubifyConfigMap[0]["MVerticesNum"] << std::endl;
            }
		}

		if(str.find("MaxStorageNumTuples") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                std::string maxStorageNumTuples = str.substr(pos+1);
                boost::trim(maxStorageNumTuples);
                cubifyConfigMap[queryID]["MaxStorageNumTuples"] = maxStorageNumTuples;
                //std::cout << "cubifyConfigMap[queryID][MVerticesNum]: " << cubifyConfigMap[0]["MVerticesNum"] << std::endl;
            }
		}

		if(str.find("ReadToWriteCostRatio") != std::string::npos)
        {
            std::size_t pos = str.find('=');

            // if "=" found
            if(pos != std::string::npos)
            {
                std::string readToWriteCostRatio = str.substr(pos+1);
                boost::trim(readToWriteCostRatio);
                cubifyConfigMap[queryID]["ReadToWriteCostRatio"] = readToWriteCostRatio;
                //std::cout << "cubifyConfigMap[queryID][MVerticesNum]: " << cubifyConfigMap[0]["MVerticesNum"] << std::endl;
            }
		}
	}
}

void OLAPManager::assignFrequency(int queryID, std::string RFMode)
{
    //std::cout << "RFMode " << RFMode << std::endl;
    //#RFMode (Ref Frequency Mode): Rand, AllHigh, AllLow, CoarseHigh, FineHigh, OneDimHigh, Custom
    int maxFrequency = 1000;
    int allHighFreqLowerBound = 0.75*maxFrequency;
    int allLowFreqUpperBound = 0.25*maxFrequency;
    int coarseHighRandMin = 0.9*maxFrequency;
    int fineHighRandMin = 0.1*maxFrequency;
    int coarseFineFreqStep = 0.1*maxFrequency;
    int oneDimHighUpperBound = 0.5*maxFrequency;


    int numVertices = pow(2, this->dimensions.size());
    srand (time(NULL));

    if(RFMode == "Rand")
    {
		int freqArr[65] = {580, 730, 850, 40, 10, 130, 720, 820, 510, 180, 970, 60, 580, 600, 610, 270, 710, 830, 280, 550, 560, 780, 500, 570, 890, 430, 390, 130, 270, 950, 870, 530, 130, 290, 630, 160, 560, 370, 500, 740, 460, 520, 430, 600, 450, 180, 120, 420, 920, 10, 650, 170, 920, 240, 640, 610, 480, 810, 190, 850, 350, 700, 670, 980, 400};
		for(int i = 0; i < numVertices; i++)
        {
            int randInt = freqArr[i];
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }

        /*
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = 1 + (rand() % (int)(maxFrequency));
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
        */
    }
    else if(RFMode == "AllHigh")
    {
		int freqArr[65] = {780, 730, 850, 940, 990, 830, 920, 880, 810, 980, 970, 760, 910, 800, 910, 870, 790, 895, 880, 980, 960, 780, 900, 870, 990, 930, 790, 830, 970, 750, 870, 930, 830, 790, 830, 760, 960, 870, 900, 790, 860, 820, 930, 900, 850, 980, 820, 920, 890, 810, 850, 870, 920, 940, 840, 910, 780, 810, 790, 850, 950, 800, 770, 980, 800};
		for(int i = 0; i < numVertices; i++)
        {
            int randInt = freqArr[i];
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
        /*
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = allHighFreqLowerBound + (rand() % (int)(allLowFreqUpperBound));
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
        */
    }
    else if(RFMode == "AllLow")
    {
		int freqArr[65] = {180, 30, 150, 240, 100, 130, 220, 120, 10, 180, 170, 60, 210, 100, 210, 120, 110, 230, 80, 150, 160, 80, 200, 170, 90, 130, 90, 230, 170, 50, 170, 30, 130, 90, 130, 160, 60, 170, 100, 240, 160, 120, 230, 200, 150, 180, 20, 220, 30, 10, 50, 170, 120, 240, 140, 110, 80, 180, 190, 50, 250, 200, 70, 180, 200};
		for(int i = 0; i < numVertices; i++)
        {
            int randInt = freqArr[i];
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
        /*
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = 1 + (rand() % (int)(allLowFreqUpperBound));
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
        */
    }
    else if(RFMode == "CoarseHigh")
    {
		int freqArr[65] = {990, 980, 970, 950, 930, 920, 910, 900, 890, 880, 870, 860, 850, 830, 810, 800, 790, 780, 750, 730, 700, 680, 650, 630, 620, 600, 590, 570, 550, 530, 500, 480, 430, 420, 410, 400, 390, 370, 360, 340, 330, 320, 310, 300, 290, 280, 270, 260, 250, 240, 230, 220, 210, 200, 180, 170, 160, 150, 140, 130, 120, 110, 100, 90, 40};
		for(int i = 0; i < numVertices; i++)
        {
            int randInt = freqArr[i];
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
        /*
        double randMin = coarseHighRandMin;
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = (int)randMin + (rand() % (int)(coarseFineFreqStep));
            randMin -= (maxFrequency/numVertices);
            std::stringstream randomIntSS;
            randomIntSS << abs(randInt);
            refFrequencies.push_back(randomIntSS.str());
            //std::cout << "Assigned Freq. | randMin " << randomIntSS.str() << " | " << randMin << std::endl;
        }
        */
    }
    else if(RFMode == "FineHigh")
    {
		int freqArr[65] = {40, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 360, 370, 390, 400, 410, 420, 430, 480, 500, 530, 550, 570, 590, 600, 620, 630, 650, 680, 700, 730, 750, 780, 790, 800, 810, 830, 850, 860, 870, 880, 890, 900, 910, 920, 930, 950, 970, 980, 990};
		for(int i = 0; i < numVertices; i++)
        {
            int randInt = freqArr[i];
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
        /*
        double randMin = fineHighRandMin;
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = (int)randMin + (rand() % (int)(coarseFineFreqStep));
            randMin += (maxFrequency/numVertices);
            std::stringstream randomIntSS;
            randomIntSS << abs(randInt);
            refFrequencies.push_back(randomIntSS.str());
            //std::cout << "Assigned Freq. | randMin " << randomIntSS.str() << " | " << randMin << std::endl;
        }
        */
    }
    else if(RFMode == "OneDimHigh")
    {
		int freqArr[65] = {30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 800, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10, 30, 15, 20, 40, 10};
		for(int i = 0; i < numVertices; i++)
        {
            int randInt = freqArr[i];
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }

		/*
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = 1 + (rand() % (int)(oneDimHighUpperBound));
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }

        int randomHighVertex = 1 + (rand() % (int)(numVertices-2)); // Selecting one vertex randomly to keep high
        std::stringstream maxFreq;
        maxFreq << maxFrequency;
        refFrequencies[randomHighVertex - 1] = maxFreq.str();
        //std::cout << "Chosen vertex " << randomHighVertex << std::endl;
        */
    }
    else
    {
        std::cout << "Invalid Cubify RFMode" << std::endl;
        exit(0);
    }

    /*
    if(RFMode == "Rand")
    {
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = 1 + (rand() % (int)(maxFrequency));
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
            //std::cout << "Assigned Freq. " << randomIntSS.str() << std::endl;
        }
    }
    else if(RFMode == "AllHigh")
    {
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = allHighFreqLowerBound + (rand() % (int)(allLowFreqUpperBound));
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
    }
    else if(RFMode == "AllLow")
    {
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = 1 + (rand() % (int)(allLowFreqUpperBound));
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }
    }
    else if(RFMode == "CoarseHigh")
    {
        double randMin = coarseHighRandMin;
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = (int)randMin + (rand() % (int)(coarseFineFreqStep));
            randMin -= (maxFrequency/numVertices);
            std::stringstream randomIntSS;
            randomIntSS << abs(randInt);
            refFrequencies.push_back(randomIntSS.str());
            //std::cout << "Assigned Freq. | randMin " << randomIntSS.str() << " | " << randMin << std::endl;
        }
    }
    else if(RFMode == "FineHigh")
    {
        double randMin = fineHighRandMin;
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = (int)randMin + (rand() % (int)(coarseFineFreqStep));
            randMin += (maxFrequency/numVertices);
            std::stringstream randomIntSS;
            randomIntSS << abs(randInt);
            refFrequencies.push_back(randomIntSS.str());
            //std::cout << "Assigned Freq. | randMin " << randomIntSS.str() << " | " << randMin << std::endl;
        }
    }
    else if(RFMode == "OneDimHigh")
    {
        for(int i = 0; i < numVertices; i++)
        {
            int randInt = 1 + (rand() % (int)(oneDimHighUpperBound));
            std::stringstream randomIntSS;
            randomIntSS << randInt;
            refFrequencies.push_back(randomIntSS.str());
        }

        int randomHighVertex = 1 + (rand() % (int)(numVertices-2)); // Selecting one vertex randomly to keep high
        std::stringstream maxFreq;
        maxFreq << maxFrequency;
        refFrequencies[randomHighVertex - 1] = maxFreq.str();
        //std::cout << "Chosen vertex " << randomHighVertex << std::endl;
    }
    else
    {
        std::cout << "Invalid Cubify RFMode" << std::endl;
        exit(0);
    }
    */
}

void OLAPManager::generateLatticeVertices(std::vector<std::string> dimensionVector)
{
    vertexInfo vInfo;
    std::vector<std::string> vertexDimensions;

    std::vector<int> v;
    int n = dimensionVector.size();

    for(int i = 0; i < pow(2, n); i++)
    {
        int m = i;
        int vertexLevelCounter = 0;
        int vertexOrdinal = 0;
        int vertexMaxRows = 1;

        while(m>1)
        {
            v.push_back(m%2);
            m = m / 2;
        }
        v.push_back(m);

        for(int k = 0; k < v.size(); ++k)
        {
            if(v[k])
            {
                vertexDimensions.push_back(dimensionVector[k]);
                vertexMaxRows *= atoi(dimensionSizes[k].c_str());
                vertexLevelCounter++;
            }
        }

        // Find which nodes to materialize based on vertices provided to materialize in cubify.conf file
        /*
        vInfo.isMaterialized = false;
        for(mVerticesIt = mVertices.begin(); mVerticesIt != mVertices.end(); mVerticesIt++)
        {
            // If already materialized
            if(vInfo.isMaterialized)
                break;

            // Always materialize most granular node
            if(dimensionVector.size() == vertexDimensions.size())
                vInfo.isMaterialized = true;

            std::vector<std::string> tmpVector = *mVerticesIt;

            // Compare only if two vectors are of same length
            if(tmpVector.size() == vertexLevelCounter)
            {
                std::sort(vertexDimensions.begin(), vertexDimensions.end());
                std::sort(tmpVector.begin(), tmpVector.end());

                for(int i = 0; i < vertexLevelCounter; i++)
                {
                    if(std::equal (vertexDimensions.begin(), vertexDimensions.end(), tmpVector.begin()) )
                    {
                        vInfo.isMaterialized = true;
                        break;
                    }
                }
            }
        }
        */
        // ~ Find which nodes to materialize

        // For ordinal numbering
        for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
        {
            if(latticeVerticesIt->first.level == vertexLevelCounter)
            {
                vertexOrdinal = latticeVerticesIt->first.ordinal;
                vertexOrdinal++;
            }
        }
        if(vertexOrdinal == 0)
            vertexOrdinal = 1;
        // ~For ordinal numbering

        vInfo.isMaterialized = false; // all the vertices are not materialized initially
        vInfo.level = vertexLevelCounter;
        vInfo.ordinal = vertexOrdinal;
        vInfo.vRows = vertexMaxRows;

        // No need to include level 0 node
        if(vertexLevelCounter!=0)
        {
            latticeVertices.insert(std::make_pair(vInfo, vertexDimensions));
            //for(std::vector<std::string>::iterator tmpIt = vertexDimensions.begin(); tmpIt != vertexDimensions.end(); tmpIt++)
                //std::cout << *tmpIt << " | ";
            //std::cout << "ordinal " << vInfo.ordinal << " | level " << vInfo.level << std::endl;
            //std::cout << std::endl;
        }

        vertexOrdinal = 0;
        v.clear();
        vertexDimensions.clear();
    }

    //std::cout << "latticeVertices size " << latticeVertices.size() << std::endl;
    //Assigning vertex IDs and refFrequencies

    int vertexIDCounter = 1;
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        //std::cout << counter++ << std::endl;
        //Why the loop iterating for 30 times instead of 31 times??
        //std::cout << "vertexIDCounter " << vertexIDCounter << std::endl;

        //vertexInfo vInfo = latticeVerticesIt->first;
        //std::vector<std::string> vDims = latticeVerticesIt->second;
        //vInfo.vertexID = vertexIDCounter;

        //latticeVerticesIt->first = vInfo;
        latticeVerticesIt->first.vertexID = vertexIDCounter;

        //std::cout << vInfo.vertexID << std::endl;
        //for(std::vector<std::string>::iterator tmpIt = vDims.begin(); tmpIt != vDims.end(); tmpIt++)
                //std::cout << *tmpIt << std::endl;

        if(isnan(atoi(refFrequencies[vertexIDCounter - 1].c_str())))
        {
            std::cout << "Invalid Ref. Frequency!" << std::endl;
            exit(0);
        }
        else
        {
            //vInfo.refFrequency = atoi(refFrequencies[vertexIDCounter - 1].c_str());
            latticeVerticesIt->first.refFrequency = atoi(refFrequencies[vertexIDCounter - 1].c_str());
        }

        vertexIDCounter++;

        //latticeVertices.erase(latticeVerticesIt);
        //latticeVertices.insert(std::make_pair(vInfo, vDims));
    }

    //for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    //{
    //    latticeVertices.insert(latticeVerticesIt);
    //}
}

/*
std::vector<std::string> OLAPManager::getCubifyDimensions()
{
    return this->cubifyDimensions;
}

std::vector< std::vector<std::string> > OLAPManager::getCubifyMVertices()
{
    return this->cubifyMVertices;
}
*/

void OLAPManager::markVerticesToMaterializeThruOptScheme(int queryID)
{
    int numVerticesToMaterialize = atoi(cubifyConfigMap[queryID]["MVerticesNum"].c_str());
    int maxStorageNumTuples = atoi(cubifyConfigMap[queryID]["MaxStorageNumTuples"].c_str());
    int IoA = atoi(cubifyConfigMap[queryID]["IoA"].c_str());
    int readToWriteCostRatio = atoi(cubifyConfigMap[queryID]["ReadToWriteCostRatio"].c_str());
    std::string optMethod = cubifyConfigMap[queryID]["OptMethod"];
    std::string timeGrain = cubifyConfigMap[queryID]["TimeGrain"];

    // Marking the most granular vertex to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Always materialize most granular node
        if(latticeVerticesIt->first.level == this->dimensions.size())
        {
            latticeVerticesIt->first.isMaterialized = true;
            break;
        }
    }

    int numOfChunks = getNumOfChunks(IoA, timeGrain);

    if(numOfChunks == -1)
    {
        std::cout << "OLAPManager: Invalid number of chunks." << std::endl;
        assert(false);
        exit(0);
    }

    //optMode numVertices | maxStorage
    StreamOLAPOptimization::getInstance()->getOptimizedVerticesToMaterialize(latticeVertices, 300, 1000, optMethod, maxStorageNumTuples, numVerticesToMaterialize, queryID, IoA, readToWriteCostRatio, numOfChunks);
    // Testing materialized nodes
    //printVertices();
}

int OLAPManager::getNumOfChunks(int IoA, std::string timeGrain)
{
    int numOfChunks = -1;

    if (timeGrain == "Second" || timeGrain == "second")
    {
        numOfChunks = IoA;
    }
    else if (timeGrain == "Minute" || timeGrain == "minute")
    {
        numOfChunks = ceil ((IoA*1.0)/60);
    }
    else if (timeGrain == "Hour" || timeGrain == "hour")
    {
        numOfChunks = ceil ((IoA*1.0)/(60*60));
    }
    else
    {
        assert(false);
    }

    return numOfChunks;
}

void OLAPManager::printVertices()
{
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Print only materialized cubes
        //if(latticeVerticesIt->first.isMaterialized)
        {
            if(latticeVerticesIt->first.isMaterialized)
                std::cout << "************ Materizlied Node ************" << std::endl;

            std::cout << "VertexID: " << latticeVerticesIt->first.vertexID << " | Ref. Freq.: " << latticeVerticesIt->first.refFrequency<< std::endl;
            //std::cout << "Ref Freq." << ", " << latticeVerticesIt->first.refFrequency << std::endl;
            //std::cout << latticeVerticesIt->first.level << ", " << latticeVerticesIt->first.ordinal << std::endl;
            std::vector<std::string> tmpVector = latticeVerticesIt->second;
            std::cout << "Max Rows: " << latticeVerticesIt->first.vRows << std::endl;

            for(std::vector<std::string>::iterator tmpIt = tmpVector.begin(); tmpIt != tmpVector.end(); tmpIt++)
                std::cout << *tmpIt << ", ";

            std::cout << std::endl << std::endl;
        }
    }
}

void OLAPManager::markNVerticesToMaterializeRandomly(int queryID)
{
    int numVerticesToMaterialize = atoi(cubifyConfigMap[queryID]["MVerticesNum"].c_str());
    int maxStorageNumTuples = atoi(cubifyConfigMap[queryID]["MaxStorageNumTuples"].c_str());

    numVerticesMaterialized = numVerticesToMaterialize;

    std::vector<int> MVertexIDs;
    std::vector<int>::iterator MVertexIDsIt;
    int randomVertexID;
    int storageConsumed = 0;
    int verticesMarkedForMaterialization = 0;

    srand(time(NULL));

    while(MVertexIDs.size() < numVerticesToMaterialize)
    {
        randomVertexID = (rand() % (latticeVertices.size() - 1)) + 1; // +1 since it generates zero based
        MVertexIDsIt = std::find (MVertexIDs.begin(), MVertexIDs.end(), randomVertexID);

        if(MVertexIDsIt == MVertexIDs.end()) // if nothing found
            MVertexIDs.push_back(randomVertexID);
    }

    // Marking the most granular vertex to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Always materialize most granular node
        if(latticeVerticesIt->first.level == this->dimensions.size())
        {
            latticeVerticesIt->first.isMaterialized = true;
            continue;
        }
    }

    // Marking the vertices to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        MVertexIDsIt = std::find (MVertexIDs.begin(), MVertexIDs.end(), latticeVerticesIt->first.vertexID);

        if(MVertexIDsIt != MVertexIDs.end()) // if found
        {
            storageConsumed += latticeVerticesIt->first.vRows;
            if(storageConsumed > maxStorageNumTuples)
            {
                //std::cout << "Random Approach: Storage Consumed exceeded the Allowed Storage" << std::endl;
                break; // Break the for loop if the total storage consumed becomes larger than available storage
            }

            verticesMarkedForMaterialization++;
            if(verticesMarkedForMaterialization > numVerticesToMaterialize)
                break;

            latticeVerticesIt->first.isMaterialized = true;
        }
    }

    //testing loop
    //printVertices();
}

void OLAPManager::markVerticesToMaterializeRandomly(int queryID)
{
    //int numVerticesToMaterialize = atoi(cubifyConfigMap[queryID]["MVerticesNum"].c_str());
    int maxStorageNumTuples = atoi(cubifyConfigMap[queryID]["MaxStorageNumTuples"].c_str());
    numVerticesMaterialized = 0;

    std::vector<int> MVertexIDs;
    std::vector<int>::iterator MVertexIDsIt;
    int randomVertexID;
    int storageConsumed = 0;
    int verticesMarkedForMaterialization = 0;

    srand(time(NULL));

    while(MVertexIDs.size() < (latticeVertices.size() - 1) ) // break if both the vertices are equal
    {
        randomVertexID = (rand() % (latticeVertices.size() - 1)) + 1; // +1 since it generates zero based
        MVertexIDsIt = std::find (MVertexIDs.begin(), MVertexIDs.end(), randomVertexID);

        if(MVertexIDsIt == MVertexIDs.end()) // if not already in the list of materialized vertices
            MVertexIDs.push_back(randomVertexID);
    }

    // Marking the most granular vertex to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Always materialize most granular node
        if(latticeVerticesIt->first.level == this->dimensions.size())
        {
            latticeVerticesIt->first.isMaterialized = true;
            numVerticesMaterialized++;
            continue;
        }
    }

    // Marking the vertices to materialize
    for(MVertexIDsIt = MVertexIDs.begin(); MVertexIDsIt != MVertexIDs.end(); MVertexIDsIt++)
    {
        latticeVerticesIt = getLatticeVertexByID( (*MVertexIDsIt) );

        if(latticeVerticesIt->first.isMaterialized == true) // ignoring the finest vertex already materialized
            continue;

        if(latticeVerticesIt != latticeVertices.end()) // if found
        {
            storageConsumed += latticeVerticesIt->first.vRows;
            if(storageConsumed > maxStorageNumTuples)
            {
                //std::cout << "Random Approach: Storage Consumed exceeded the Allowed Storage" << std::endl;
                break; // Break the for loop if the total storage consumed becomes larger than available storage
            }

            latticeVerticesIt->first.isMaterialized = true;
            numVerticesMaterialized++;
        }
    }

    //testing loop
    //printVertices();
}


void OLAPManager::markSmallestVerticesToMaterialize(int queryID)
{
    int maxStorageNumTuples = atoi(cubifyConfigMap[queryID]["MaxStorageNumTuples"].c_str());
    std::multimap<int, int> vertexIDSizeMap;
    std::multimap<int, int>::iterator vertexIDSizeMapIt;
    //std::multimap<int, int>::reverse_iterator refFreqVertexIDMapRIt;

    std::vector<int> MVertexIDs;
    std::vector<int>::iterator MVertexIDsIt;
    int storageConsumed = 0;
    numVerticesMaterialized = 0;

    // Mapping the vertexIDs and their rows (Since rows are they keys, they will be arranged in ascending ording on insertion)
    int verticesMarkedForMaterialization = 0;
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        vertexIDSizeMap.insert(std::make_pair(latticeVerticesIt->first.vRows, latticeVerticesIt->first.vertexID));
    }

    // Collecting the smallest vertices
    for(vertexIDSizeMapIt = vertexIDSizeMap.begin(); vertexIDSizeMapIt != vertexIDSizeMap.end(); vertexIDSizeMapIt++)
    {
        //std::cout << refFreqVertexIDMapRIt->first << " | " << refFreqVertexIDMapRIt->second << std::endl;
        if(MVertexIDs.size() <= latticeVertices.size()) // push all the vertices
            MVertexIDs.push_back(vertexIDSizeMapIt->second);
        else
            break;
    }

     // Marking the most granular vertex to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Always materialize most granular node
        if(latticeVerticesIt->first.level == this->dimensions.size())
        {
            latticeVerticesIt->first.isMaterialized = true;
            numVerticesMaterialized++;
            continue;
        }
    }

    // Marking the vertices to materialize
    for(MVertexIDsIt = MVertexIDs.begin(); MVertexIDsIt != MVertexIDs.end(); MVertexIDsIt++)
    {
        latticeVerticesIt = getLatticeVertexByID( (*MVertexIDsIt) );

        if(latticeVerticesIt->first.isMaterialized == true) // ignoring the finest vertex already materialized
            continue;

        if(latticeVerticesIt != latticeVertices.end()) // if found
        {
            storageConsumed += latticeVerticesIt->first.vRows;
            if(storageConsumed > maxStorageNumTuples)
            {
                break; // Break the for loop if the total storage consumed becomes larger than available storage
            }

            latticeVerticesIt->first.isMaterialized = true;
            numVerticesMaterialized++;
        }
    }

    //testing loop
    //printVertices();
}


void OLAPManager::markLargestVerticesToMaterialize(int queryID)
{
    int maxStorageNumTuples = atoi(cubifyConfigMap[queryID]["MaxStorageNumTuples"].c_str());
    std::multimap<int, int> vertexIDSizeMap;
    std::multimap<int, int>::reverse_iterator vertexIDSizeMapRIt;

    std::vector<int> MVertexIDs;
    std::vector<int>::iterator MVertexIDsIt;
    int storageConsumed = 0;
    numVerticesMaterialized = 0;

    // Mapping the vertexIDs and their rows (Since rows are they keys, they will be arranged in desending ording on insertion)
    int verticesMarkedForMaterialization = 0;
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        vertexIDSizeMap.insert(std::make_pair(latticeVerticesIt->first.vRows, latticeVerticesIt->first.vertexID));
    }

    int smallestVertexSize = INT_MAX;
    // Arranging vertices in the desending order of size (larger to smaller)
    for(vertexIDSizeMapRIt = vertexIDSizeMap.rbegin(); vertexIDSizeMapRIt != vertexIDSizeMap.rend(); vertexIDSizeMapRIt++)
    {

        if(vertexIDSizeMapRIt->first < smallestVertexSize)
            smallestVertexSize = vertexIDSizeMapRIt->first;

        //std::cout << refFreqVertexIDMapRIt->first << " | " << refFreqVertexIDMapRIt->second << std::endl;
        if(MVertexIDs.size() <= latticeVertices.size()) // push all the vertices
            MVertexIDs.push_back(vertexIDSizeMapRIt->second);
        else
            break;
    }

     // Marking the most granular vertex to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Always materialize most granular node
        if(latticeVerticesIt->first.level == this->dimensions.size())
        {
            latticeVerticesIt->first.isMaterialized = true;
            numVerticesMaterialized++;
            continue;
        }
    }

    //std::cout << "smallest vertex size " << smallestVertexSize << std::endl;

    // Marking the vertices to materialize
    for(MVertexIDsIt = MVertexIDs.begin(); MVertexIDsIt != MVertexIDs.end(); MVertexIDsIt++)
    {
        latticeVerticesIt = getLatticeVertexByID( (*MVertexIDsIt) );

        //std::cout << *MVertexIDsIt << std::endl;
        if(latticeVerticesIt != latticeVertices.end()) // if found
        {
            if(latticeVerticesIt->first.isMaterialized == true) // ignoring the finest vertex already materialized
                continue;

            storageConsumed += latticeVerticesIt->first.vRows;

            if(storageConsumed > maxStorageNumTuples)
            {
                break;
                /*
                if( (maxStorageNumTuples + latticeVerticesIt->first.vRows - storageConsumed ) > smallestVertexSize )
                {
                    storageConsumed -= latticeVerticesIt->first.vRows; // reversing the effect
                    continue;
                }
                else
                    break; // Break the for loop if the total storage consumed becomes larger than available storage
                */
            }

            latticeVerticesIt->first.isMaterialized = true;
            numVerticesMaterialized++;
        }
    }

    //testing loop
    //printVertices();
}


std::map<vertexInfo, std::vector<std::string> >::iterator OLAPManager::getLatticeVertexByID(int VID)
{
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        if(latticeVerticesIt->first.vertexID == VID)
            return latticeVerticesIt;
    }

    return latticeVertices.end();
}

void OLAPManager::markNVerticesToMaterializeThruRefFreq(int queryID)
{
    std::vector<std::string> refFrequencies;
    OLAPManager::getInstance()->getConfigValue(queryID, "RefFrequencies", refFrequencies);
    int numVerticesToMaterialize = atoi(cubifyConfigMap[queryID]["MVerticesNum"].c_str());
    int maxStorageNumTuples = atoi(cubifyConfigMap[queryID]["MaxStorageNumTuples"].c_str());
    std::multimap<int, int> refFreqVertexIDMap;
    std::multimap<int, int>::iterator refFreqVertexIDMapIt;
    std::multimap<int, int>::reverse_iterator refFreqVertexIDMapRIt;

    std::vector<int> MVertexIDs;
    std::vector<int>::iterator MVertexIDsIt;
    int randomVertexID;
    int storageConsumed = 0;

    numVerticesMaterialized = numVerticesToMaterialize;

    // Mapping the vertexIDs and their frequencies
    int i = 0;
    int verticesMarkedForMaterialization = 0;
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++, i++)
    {
        refFreqVertexIDMap.insert(std::make_pair(atoi(refFrequencies[i].c_str()), latticeVerticesIt->first.vertexID));
    }

    // Collecting the most frequently queried vertices
    for(refFreqVertexIDMapRIt = refFreqVertexIDMap.rbegin(); refFreqVertexIDMapRIt != refFreqVertexIDMap.rend(); refFreqVertexIDMapRIt++)
    {
        //std::cout << refFreqVertexIDMapRIt->first << " | " << refFreqVertexIDMapRIt->second << std::endl;
        if(MVertexIDs.size() <= numVerticesToMaterialize)
            MVertexIDs.push_back(refFreqVertexIDMapRIt->second);
        else
            break;
    }

     // Marking the most granular vertex to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Always materialize most granular node
        if(latticeVerticesIt->first.level == this->dimensions.size())
        {
            latticeVerticesIt->first.isMaterialized = true;
            continue;
        }
    }

    // Marking the vertices to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        MVertexIDsIt = std::find (MVertexIDs.begin(), MVertexIDs.end(), latticeVerticesIt->first.vertexID);

        if(MVertexIDsIt != MVertexIDs.end()) // if found
        {
            storageConsumed += latticeVerticesIt->first.vRows;
            if(storageConsumed > maxStorageNumTuples)
            {
                //std::cout << "Frequency Approach: Storage Consumed exceeded the Allowed Storage" << std::endl;
                break; // Break the for loop if the total storage consumed becomes larger than available storage
            }

            // Counting the vertices already marked for materialization
            verticesMarkedForMaterialization++;
            if(verticesMarkedForMaterialization > numVerticesToMaterialize)
                break;

            latticeVerticesIt->first.isMaterialized = true;
        }
    }

    //testing loop
    //printVertices();
}


void OLAPManager::markVerticesToMaterializeThruRefFreq(int queryID)
{
    std::vector<std::string> refFrequencies;
    OLAPManager::getInstance()->getConfigValue(queryID, "RefFrequencies", refFrequencies);
    //int numVerticesToMaterialize = atoi(cubifyConfigMap[queryID]["MVerticesNum"].c_str());
    int maxStorageNumTuples = atoi(cubifyConfigMap[queryID]["MaxStorageNumTuples"].c_str());
    std::multimap<int, int> refFreqVertexIDMap;
    std::multimap<int, int>::iterator refFreqVertexIDMapIt;
    std::multimap<int, int>::reverse_iterator refFreqVertexIDMapRIt;

    std::vector<int> MVertexIDs;
    std::vector<int>::iterator MVertexIDsIt;
    int randomVertexID;
    int storageConsumed = 0;
    numVerticesMaterialized = 0;

    // Mapping the vertexIDs and their frequencies
    int i = 0;
    int verticesMarkedForMaterialization = 0;
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++, i++)
    {
        refFreqVertexIDMap.insert(std::make_pair(atoi(refFrequencies[i].c_str()), latticeVerticesIt->first.vertexID));
    }

    // Collecting the most frequently queried vertices
    for(refFreqVertexIDMapRIt = refFreqVertexIDMap.rbegin(); refFreqVertexIDMapRIt != refFreqVertexIDMap.rend(); refFreqVertexIDMapRIt++)
    {
        //std::cout << refFreqVertexIDMapRIt->first << " | " << refFreqVertexIDMapRIt->second << std::endl;
        if(MVertexIDs.size() <= latticeVertices.size()) // push all the vertices
            MVertexIDs.push_back(refFreqVertexIDMapRIt->second);
        else
            break;
    }

     // Marking the most granular vertex to materialize
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Always materialize most granular node
        if(latticeVerticesIt->first.level == this->dimensions.size())
        {
            latticeVerticesIt->first.isMaterialized = true;
            numVerticesMaterialized++;
            continue;
        }
    }

    // Marking the vertices to materialize
    for(MVertexIDsIt = MVertexIDs.begin(); MVertexIDsIt != MVertexIDs.end(); MVertexIDsIt++)
    {
        latticeVerticesIt = getLatticeVertexByID( (*MVertexIDsIt) );

        if(latticeVerticesIt->first.isMaterialized == true) // ignoring the finest vertex already materialized
            continue;

        if(latticeVerticesIt != latticeVertices.end()) // if found
        {
            storageConsumed += latticeVerticesIt->first.vRows;
            if(storageConsumed > maxStorageNumTuples)
            {
                //std::cout << "Frequency Approach: Storage Consumed exceeded the Allowed Storage" << std::endl;
                break; // Break the for loop if the total storage consumed becomes larger than available storage
            }

            latticeVerticesIt->first.isMaterialized = true;
            numVerticesMaterialized++;
        }
    }
    //testing loop
    //printVertices();
}

void OLAPManager::markVerticesToMaterializeThruConfigFile()
{
    numVerticesMaterialized = 0;

    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        //vertexInfo vInfo = latticeVerticesIt->first;
        //vertexInfo newVInfo = vInfo;
        //std::cout << "vInfo.ordinal " << vInfo.ordinal << std::endl;
        //std::cout << "vInfo.level " << vInfo.level << std::endl;
        //std::cout << "latticeVerticesIt->second.size() " << latticeVerticesIt->second.size() << std::endl;

        for(mVerticesIt = mVertices.begin(); mVerticesIt != mVertices.end(); mVerticesIt++)
        {
            //std::cout << "vInfo.isMaterialized " << vInfo.isMaterialized << std::endl;
            // If already materialized
            if(latticeVerticesIt->first.isMaterialized)
                break;

            std::vector<std::string> tmpVector = *mVerticesIt;
            std::vector<std::string> vertexDimensions = latticeVerticesIt->second;
            //std::vector<std::string> newVertexDimensions = latticeVerticesIt->second;

            // Always materialize most granular node
            //if(vInfo.level == latticeVerticesIt->second.size())
            if(latticeVerticesIt->first.level == this->dimensions.size())
            {
                latticeVerticesIt->first.isMaterialized = true;
                numVerticesMaterialized++;
                //newVInfo.isMaterialized = true;
            }

            // Compare only if two vectors are of same length
            if(tmpVector.size() == latticeVerticesIt->first.level)
            {
                std::sort(vertexDimensions.begin(), vertexDimensions.end());
                std::sort(tmpVector.begin(), tmpVector.end());

                for(int i = 0; i < latticeVerticesIt->first.level; i++)
                {
                    if(std::equal (vertexDimensions.begin(), vertexDimensions.end(), tmpVector.begin()) )
                    {
                        //newVInfo.isMaterialized = true;
                        latticeVerticesIt->first.isMaterialized = true;
                        numVerticesMaterialized++;
                        break;
                    }
                }
            }
            //latticeVertices.erase(latticeVerticesIt);
            //latticeVertices.insert(std::make_pair(newVInfo, newVertexDimensions));
        }
    }

    //testing loop
    //printVertices();
}

//void OLAPManager::getLatticeVertices(std::string configFile, std::map<vertexInfo, std::vector<std::string> >& latVertices)
std::map<vertexInfo, std::vector<std::string> > OLAPManager::getLatticeVertices(std::string configFile, int queryID)
{
    this->latticeVertices.clear();
    this->mVertices.clear();
    this->dimensions.clear();

    readCubifyConfiguration(configFile, queryID);
    // dimensions vector is populated by readCubifyConfiguration function (above)
    generateLatticeVertices(dimensions);

    std::string MMode = getConfigValue(queryID, "MMode");
    std::string optMethod = cubifyConfigMap[queryID]["OptMethod"];

    if(MMode == "Optimized")
        markVerticesToMaterializeThruOptScheme(queryID);
    else if (MMode == "Random" && optMethod == "NumVertices")
        markNVerticesToMaterializeRandomly(queryID);
    else if (MMode == "Random" && optMethod == "MaxStorage")
        markVerticesToMaterializeRandomly(queryID);
    else if (MMode == "Frequency" && optMethod == "NumVertices")
        markNVerticesToMaterializeThruRefFreq(queryID);
    else if (MMode == "Frequency" && optMethod == "MaxStorage")
        markVerticesToMaterializeThruRefFreq(queryID);
    else if (MMode == "SmallestVertices")
        markSmallestVerticesToMaterialize(queryID);
    else if (MMode == "LargestVertices")
        markLargestVerticesToMaterialize(queryID);
    else if (MMode == "Custom")
        markVerticesToMaterializeThruConfigFile();
    else
    {
        std::cout << "Invalid Materialization Mode" << std::endl;
        exit(0);
    }

    queryIDLatticeVerticesMap.insert(make_pair(queryID, this->latticeVertices));
    return this->latticeVertices;
    //latVertices = this->latticeVertices;
}

std::string OLAPManager::getConfigValue(int queryID, std::string key)
{
    if( (cubifyConfigMapIt = cubifyConfigMap.find(queryID)) != cubifyConfigMap.end())
    {
        std::map<std::string, std::string>::iterator cubifyConfigMapInnerIt = cubifyConfigMapIt->second.find(key);

        if(cubifyConfigMapInnerIt != cubifyConfigMapIt->second.end())
            return cubifyConfigMapInnerIt->second;
        //else
            //std::cout << "OLAPManager: Given key: " << key << ", does not exist." << std::endl;
    }
}

void OLAPManager::getConfigValue(int queryID, std::string key, std::vector<std::string>& configValueVector)
{
    if( ( cubifyConfigVectorMapIt = cubifyConfigVectorMap.find(queryID) ) != cubifyConfigVectorMap.end() )
    {
        if( (cubifyConfigVectorMapInnerIt = cubifyConfigVectorMapIt->second.find(key)) != cubifyConfigVectorMapIt->second.end() )
            configValueVector = cubifyConfigVectorMapInnerIt->second;
        //else
            //std::cout << "OLAPManager: Given key: " << key << ", does not exist."  << std::endl;
    }
}

void OLAPManager::getConfigValue(int queryID, std::string key, std::vector< std::vector<std::string> >& configValueVector)
{
    if( ( cubifyConfigNestedVectorMapIt = cubifyConfigNestedVectorMap.find(queryID) ) != cubifyConfigNestedVectorMap.end() )
    {
        if( (cubifyConfigNestedVectorMapInnerIt = cubifyConfigNestedVectorMapIt->second.find(key)) != cubifyConfigNestedVectorMapIt->second.end() )
            configValueVector = cubifyConfigNestedVectorMapInnerIt->second;
        //else
            //std::cout << "OLAPManager: Given key: " << key << ", does not exist."  << std::endl;
    }
}

int OLAPManager::getNumOfMaterializedVertices(int queryID)
{
    int counter = 0;
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        if(latticeVerticesIt->first.isMaterialized)
            counter++;
    }
    return counter;
}

std::map<vertexInfo, std::vector<std::string> > OLAPManager::getLatticeVertices(int queryID)
{
    std::map<int, std::map<vertexInfo, std::vector<std::string> > >::iterator latticeVerticesMapIt;
    latticeVerticesMapIt = queryIDLatticeVerticesMap.find(queryID);


    if( latticeVerticesMapIt !=  queryIDLatticeVerticesMap.end() )
        return latticeVerticesMapIt->second;
    else
    {
        std::cout << "No lattice vertices with given ID found!" << std::endl;
    }
}
