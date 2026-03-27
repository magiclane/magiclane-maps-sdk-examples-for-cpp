// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include <API/GEM_ApiLists.h>
#include <API/GEM_GeographicArea.h>

#include <mutex>
#include <cstring>

class MultiSearchViewModel {
public:
    MultiSearchViewModel(const MultiSearchViewModel& obj) = delete;

    static MultiSearchViewModel* getInstance() {
        if (instancePtr == nullptr) {
            instancePtr = new MultiSearchViewModel();
        }
        return instancePtr;
    }

    enum class EViewPurpose {
        AROUND_POSITION_SEARCH = 0,
        TEXT_SEARCH = 1,
        BY_AREA_SEARCH
    };

    gem::Coordinates& getTargetPos(EViewPurpose purpose) {
        switch (purpose)
        {
        case EViewPurpose::AROUND_POSITION_SEARCH:
            return m_targetPosition;
            break;
        case EViewPurpose::TEXT_SEARCH:
            return m_targetPositionTextSearch;
            break;
        case EViewPurpose::BY_AREA_SEARCH:
            return m_targetPositionArea;
            break;
        };
        return m_targetPosition;
    };
    void setTargetPosition(EViewPurpose purpose, double positionLat, double positionLon) {
        gem::Coordinates position(positionLat, positionLon);
        if (position.valid()) {
            switch (purpose)
            {
            case EViewPurpose::AROUND_POSITION_SEARCH:
                m_targetPosition = position;
                break;
            case EViewPurpose::TEXT_SEARCH:
                m_targetPositionTextSearch = position;
                break;
            case EViewPurpose::BY_AREA_SEARCH:
                m_targetPositionArea = position;
                break;
            };
        }
    };

    char* getSearchText(EViewPurpose purpose) {
        if (purpose == EViewPurpose::TEXT_SEARCH)
            return searchText;
        return areaSearchText;
    }
    void setSearchText(EViewPurpose purpose, char* text) {
        if (purpose == EViewPurpose::TEXT_SEARCH)
            strcpy(searchText, text);
        if (purpose == EViewPurpose::TEXT_SEARCH)
            strcpy(areaSearchText, text);
    }
    int getSearchSize() {
        return 128;
    }

    gem::RectangleGeographicArea& getSearchArea() {
        return rgaLatLon;
    }
    void setSearchAreaTopLeft(double positionLat, double positionLon) {
        gem::Coordinates position(positionLat, positionLon);
        if (position.valid()) {
            rgaLatLon.setTopLeft(position);
        }
    }

    void setSearchAreaBottomRight(double positionLat, double positionLon) {
        gem::Coordinates position(positionLat, positionLon);
        if (position.valid()) {
            rgaLatLon.setBottomRight(position);
        }
    }

private:
    gem::Coordinates m_targetPosition = gem::Coordinates(25.607, -80.399);
    gem::Coordinates m_targetPositionTextSearch = gem::Coordinates(37.5503, 126.9971);
    char searchText[128] = "Tower";
    char areaSearchText[128] = "Tower";
    gem::Coordinates m_targetPositionArea = gem::Coordinates(48.858953, 2.293399);
    gem::RectangleGeographicArea rgaLatLon = gem::RectangleGeographicArea(48.973932, 48.746010, 2.511954, 2.189766); // (48.973932, 2.189766), (48.746010, 2.511954)
    static MultiSearchViewModel* instancePtr;
    MultiSearchViewModel() {}
};

