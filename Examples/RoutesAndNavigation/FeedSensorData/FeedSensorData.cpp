// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_SenseDataTypes.h>
#include <API/GEM_SenseDataSource.h>

#include "DataAcceleration.h"
#include "DataGyroscope.h"
#include "DataPosition.h"

// Define a custom DataSourceListener class to enable receiving processed data from the SDK

class DataReceiver : public gem::sense::IDataSourceListener
{
public:
    DataReceiver() {}

    // The custom DataSourceListener must implement the onNewData function to enable receiving processed data from the SDK

    void onNewData(gem::sense::DataPtr data) override
    {
        auto dataType = data.get()->getType();
        switch((int)dataType)
        {
        case (int)gem::sense::EDataType::Acceleration:
            GEM_LOGE("RECEIVED NEW DATA - Acceleration"); break;
        case (int)gem::sense::EDataType::Activity:
            GEM_LOGE("RECEIVED NEW DATA - Activity"); break;
        case (int)gem::sense::EDataType::Attitude:
            GEM_LOGE("RECEIVED NEW DATA - Attitude"); break;
        case (int)gem::sense::EDataType::Battery:
            GEM_LOGE("RECEIVED NEW DATA - Battery"); break;
        case (int)gem::sense::EDataType::Camera:
            GEM_LOGE("RECEIVED NEW DATA - Camera"); break;
        case (int)gem::sense::EDataType::Compass:
            GEM_LOGE("RECEIVED NEW DATA - Compass"); break;
        case (int)gem::sense::EDataType::MagneticField:
            GEM_LOGE("RECEIVED NEW DATA - MagneticField"); break;
        case (int)gem::sense::EDataType::Orientation:
            GEM_LOGE("RECEIVED NEW DATA - Orientation"); break;
        case (int)gem::sense::EDataType::Position:
            {
                auto dataPosition = data.get()->cast<gem::sense::IPosition>();
                double alt = dataPosition.get()->getAltitude();
                double lat = dataPosition.get()->getLatitude();
                double lon = dataPosition.get()->getLongitude();
                GEM_LOGE("RECEIVED NEW DATA - Position lon,lat( %.8f, %.8f ); alt %f (inout)", lon, lat, alt);
                break;
            }
        case (int)gem::sense::EDataType::ImprovedPosition:
        {
            auto dataImprovedPosition = data.get()->cast<gem::sense::IImprovedPosition>();
            unsigned long timeStamp = dataImprovedPosition.get()->getSatelliteTime();
            double alt = dataImprovedPosition.get()->getAltitude();
            double lat = dataImprovedPosition.get()->getLatitude();
            double lon = dataImprovedPosition.get()->getLongitude();
            double speed = dataImprovedPosition.get()->getSpeed();
            double speedLimit = dataImprovedPosition.get()->getRoadSpeedLimit();
            double heading = dataImprovedPosition.get()->getCourse();
            double horizAccuracy = dataImprovedPosition.get()->getHorizontalAccuracy();
            GEM_LOGE("RECEIVED NEW DATA - ImprovedPosition[map matched] timestamp( %lu ); lon,lat( %.8f, %.8f ); alt %dm; heading %.1f(deg); horizAcc %.1fm; speed %f(m/sec), %.1f(km/h); speed limit %.1f(m/s) (output)",
                     timeStamp, lon, lat, (int)alt, heading, horizAccuracy, speed, speed*3.6, speedLimit );
            break;
        }
        case (int)gem::sense::EDataType::Gyroscope:
            GEM_LOGE("RECEIVED NEW DATA - Gyroscope"); break;
        case (int)gem::sense::EDataType::Temperature:
            GEM_LOGE("RECEIVED NEW DATA - Temperature"); break;
        case (int)gem::sense::EDataType::Notification:
            GEM_LOGE("RECEIVED NEW DATA - Notification"); break;
        case (int)gem::sense::EDataType::MountInformation:
            GEM_LOGE("RECEIVED NEW DATA - MountInformation"); break;
        default:
            GEM_LOGE("RECEIVED NEW DATA - UNDEFINED"); break;
        }
    }
};

void pushDataThread(std::string pushDataThread) //gem::sense::DataSourcePtr externalDataSource
{
    int startTimeSec = (int)positionData[0][0];
    int accelerationIndex = 0, gyroscopeIndex = 0, positionIndex = 0;

    // Create a list of the sensor data types that will be input into the SDK

    gem::sense::DataTypeList availableDataTypes;

    availableDataTypes.push_back(gem::sense::EDataType::Position); // Position is mandatory
    availableDataTypes.push_back(gem::sense::EDataType::Acceleration);
    availableDataTypes.push_back(gem::sense::EDataType::Gyroscope);

    // Create an external data source to input the above data types into the SDK

    auto externalDataSource = gem::sense::DataSourceFactory::produceExternal( availableDataTypes );

    if( externalDataSource )
    {
        // Add a listener for the data type(s) desired to be received back from the SDK

        auto listener = gem::StrongPointerFactory<DataReceiver>();
        externalDataSource->addListener(listener, gem::sense::EDataType::Position);
        externalDataSource->addListener(listener, gem::sense::EDataType::ImprovedPosition);

        // Start the data source

        externalDataSource->start();

        while (true)
        {
            GEM_LOGE("PUSH ACCELERATION DATA (input)");
            while (true)
            {
                if (accelerationIndex >= TOTAL_ACCELERATION_RECORDS || startTimeSec < (int)accelerationData[accelerationIndex][0])
                {
                    break;
                }
                externalDataSource->pushData(
                    gem::sense::DataFactory::produceAcceleration((gem::LargeInteger)(accelerationData[accelerationIndex][0] * 1000.0),
                        (double)accelerationData[accelerationIndex][1],
                        (double)accelerationData[accelerationIndex][2],
                        (double)accelerationData[accelerationIndex][3],
                        gem::sense::EUnitOfMeasurement::G
                    )); // ios data is in G, android data is in m/s^2
                accelerationIndex++;
            }

            GEM_LOGE("PUSH GYROSCOPE DATA (input)");
            while (true)
            {
                if (gyroscopeIndex >= TOTAL_GYROSCOPE_RECORDS || startTimeSec < (int)gyroscopeData[gyroscopeIndex][0])
                {
                    break;
                }
                externalDataSource->pushData(
                    gem::sense::DataFactory::produceRotationRate((gem::LargeInteger)(gyroscopeData[gyroscopeIndex][0] * 1000.0),
                        (double)gyroscopeData[gyroscopeIndex][1],
                        (double)gyroscopeData[gyroscopeIndex][2],
                        (double)gyroscopeData[gyroscopeIndex][3]
                    ));
                gyroscopeIndex++;
            }

            while (true)
            {
                if (positionIndex >= TOTAL_POSITION_RECORDS || startTimeSec < (int)positionData[positionIndex][0])
                {
                    break;
                }
                GEM_LOGE("PUSH POSITION DATA RECORD %d t = %d sec (input) lon,lat( %.8f, %.8f ); gyro rec %d; acceleration rec %d",
                    positionIndex, startTimeSec, (double)positionData[positionIndex][3], (double)positionData[positionIndex][2], gyroscopeIndex, accelerationIndex);
                externalDataSource->pushData(
                    gem::sense::DataFactory::producePosition((gem::LargeInteger)(positionData[positionIndex][0] * 1000.0),
                        (double)positionData[positionIndex][2], // Latitude
                        (double)positionData[positionIndex][3], // Longitude
                        (double)positionData[positionIndex][4], // Altitude (m)
                        (double)positionData[positionIndex][6], // Heading (deg east of north)
                        (double)positionData[positionIndex][5]  // Speed (m/s)
                    ));
                positionIndex++;
            }

            startTimeSec++;

            // Push / feed / input one position / second to the SDK, along with all other sensor measurements during that second

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        GEM_LOGE("DATA INPUT COMPLETE");
    }
}

int main( int argc, char** argv )
{
	// Get new project API token from:
	// https://developer.magiclane.com/api/projects
	std::string projectApiToken = "";

#if defined(API_TOKEN)
	projectApiToken = std::string( API_TOKEN );
#else
	auto value = std::getenv( "GEM_TOKEN" );
	if( value != nullptr )
		projectApiToken = value;
#endif

    // Sdk objects can be created & used below this line
    Environment::SdkSession session(projectApiToken, { argc > 1 ? argv[1] : "" }); // SDK API debug logging path 

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();


    GEM_LOGE("FEED SENSOR DATA EXAMPLE   ###   ###   ###   ###");

    std::thread thread1(pushDataThread, "pushDataThread");
    while( thread1.joinable() )
    {
        WAIT_TIME_OUT( 1000 );
    }

    return 0;
}

#if ( defined(_WIN32) || defined(_WIN64) ) && !defined(__MINGW32__) && !defined(__MINGW64__)

int WINAPI WinMain( HINSTANCE hInstance, // Instance
    HINSTANCE hPrevInstance, // Previous Instance
    LPSTR lpCmdLine, // Command Line Parameters
    int nCmdShow )
{
    main( 0, nullptr );

    return 0;
}

#endif
