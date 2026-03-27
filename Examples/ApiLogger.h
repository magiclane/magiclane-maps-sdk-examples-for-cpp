// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#include <API/GEM_CallLogger.h>

#include <fstream>

class ApiLogger : public gem::IApiCallLogger
{
public:
    ApiLogger( const char* file = nullptr )
    {
        if( file )
            m_logStream = std::make_shared<std::ofstream>( file, std::ios::app );
    }

    ~ApiLogger()
    {
        if( m_logStream )
            m_logStream->close();
    }

    virtual void onLog( int logLevel, char* logText, unsigned logTextSize )
    {
        if( m_logStream )
            (*m_logStream) << std::string( logText, logTextSize ) << std::endl;
    }

    virtual int onGetLogLevel() const
    {
        return gem::ELogLevel::LogInfo;
    }

private:
    std::shared_ptr<std::ofstream> m_logStream;
};
