# MagicLane - Maps SDK for C++ demo applications

This repository holds a series of example projects using the **MagicLane - Maps SDK for C++**. More information about the API can be found on the [Documentation](https://developer.magiclane.com/docs/cpp) page.

This set of individual, use-case based projects is designed to be cloned by developers for their own use.

* [Center Map](Examples/3DScene/CenterMap) - Center the map view over a pair of coordinates
* [Map Perspective](Examples/3DScene/MapPerspective) - Control the map perspective
* [Map Style](Examples/3DScene/MapStyle) - Show a different map style
* [Map View](Examples/3DScene/MapView) - Show a map view
* [Multi Map View](Examples/3DScene/MultiMapView) - Show multiple map views
* [Points Marker](Examples/3DScene/PointsMarker) - Show a collection of points on map
* [Polygon Marker](Examples/3DScene/PolygonMarker) - Show a collection of polygons on map
* [Polyline Marker](Examples/3DScene/PolylineMarker) - Show a collection of polylines on map
* [[Interactive] Calculate Route](Examples/Interactive/CalculateRoute) - Calculate a route between two given pairs of coordinates then display it on map
* [[Interactive] Change Map Style](Examples/Interactive/ChangeMapStyle) - Show how to change map style
* [[Interactive] Content Download](Examples/Interactive/ContentDownload) - Show how to download maps, voices and map styles from the MagicLane Online Store for offline use
* [[Interactive] Export Track](Examples/Interactive/ExportTrack) - Show how to save a navigation track in `.gpx`, `.kml`, `.geojson` or `.nmea` format
* [[Interactive] GPX NMEA Playback](Examples/Interactive/GpxNmeaPlayback) - Show how to playback a pre-recorded GPS-log in NMEA format
* [[Interactive] GPX Routing Simulation](Examples/Interactive/GpxRoutingSimulation) - Show how to calculate and render a route based on a GPX track as input waypoints and start a simulated navigation on that route
* [[Interactive] Markers](Examples/Interactive/Markers) - Show how to add icons with an image / polylines / polygons to the interactive map
* [[Interactive] Multi Search](Examples/Interactive/MultiSearch) - Shows how to do a search on an interactive movable and zoomable map and flies to the first search result and also shows how to search for anything on the map with a free-form text query
* [[Interactive] Reverse Geocoding](Examples/Interactive/ReverseGeocoding) - Show how to perform a reverse geocoding search to find the positions of points of interest (POIs) near a specified position on the map
* [[Interactive] Save GPX](Examples/Interactive/SaveGPX) - Save map matched improved positions in GPX format
* [[Interactive] Simulate Navigation](Examples/Interactive/SimulateNavigation) - Calculate a route between two given pairs of coordinates then perform a simulation of navigating on it
* [[Interactive] Switch Map Perspective](Examples/Interactive/SwitchMapPerspective) - Show how to change the map perspective
* [[Interactive] VRP Optimization](Examples/Interactive/VRPOptimization) - Add an optimization with custom configuration parameters, orders with different fields set, multiple vehicles with different constraints set; display the solution on the map
* [[Interactive] Weather](Examples/Interactive/Weather) - Retrieve and display current weather, hourly forecasts, and daily forecasts for any map location
* [Search Around](Examples/PlacesAndSearch/SearchAround) - Search for nearby places using a pair of coordinates
* [Search Free Text](Examples/PlacesAndSearch/SearchFreeText) - Search for places using text
* [Search Free Text Limit By Geographic Area](Examples/PlacesAndSearch/SearchFreeTextLimitByGeographicArea) - Search for places using text. Obtain the results that fall in the given geographic area
* [Calculate Route Multi View](Examples/RoutesAndNavigation/CalculateRouteMultiView) - Calculate two routes and add them in two separate views
* [Feed Sensor Data](Examples/RoutesAndNavigation/FeedSensorData) - Show how to feed SDK with different sensors data and log map matched improved position on console
* [Finger Route](Examples/RoutesAndNavigation/FingerRoute) - Calculate a route by drawing with the finger or mouse on the map
* [Navigation LowEnd HW](Examples/RoutesAndNavigation/NavigationLowEndHW) - Configure SDK for low-end devices and start a navigation based on NMEA file
* [PlayRoute](Examples/RoutesAndNavigation/PlayRoute) - Add markers collection from route and route instruction packed geometries
* [Simulation](Examples/RoutesAndNavigation/Simulation) - Calculate a route and simulate navigating on it
* [Timezone Coordinates](Examples/Timezone/TimezoneCoordinates) - Get information about timezone from specific coordinates and timestamp
* [Timezone ID](Examples/Timezone/TimezoneId) - Get information about timezone from specific timezone ID ("Country/City") and timestamp
* [[VRP] Add Circle Territory](Examples/VRP/AddCircleTerritory) - Add a circle territory
* [[VRP] Add Customer](Examples/VRP/AddCustomer) - Add a customer to the agenda to create orders for this customer and to use them in optimizations
* [[VRP] Add Full Optimization](Examples/VRP/AddFullOptimization) - Add an optimization with custom configuration parameters, orders with different fields set, multiple vehicles with different constraints set; display the solution on the map
* [[VRP] Add Optim With Single Vehicle With Different Start/End Orders](Examples/VRP/AddOptimSingleVDiffDepartDest) - Add an optimization with a single vehicle with different departure and destination locations and display the solution on the map
* [[VRP] Add Optim With Fixed Orders Sequences](Examples/VRP/AddOptimWithFixedOrdersSequences) - Add an optimization with fixed orders sequences set (the orders from a sequence will be visited in the fixed specified order) and display the solution on the map 
* [[VRP] Add Optim With Multiple Vehicles Multiple End Departures](Examples/VRP/AddOptimWithMultiVMultiDepart) - Add an optimization with multiple vehices which start their routes from different departures and display the solution on the map
* [[VRP] Add Optim With Multiple Vehicles Multiple Destinations](Examples/VRP/AddOptimWithMultiVMultiDest) - Add an optimization with multiple vehicles which end their routes at different destinations and display the solution on the map
* [[VRP] Add Optim With Multiple Vehicles Single Departure](Examples/VRP/AddOptimWithMultiVSingleDepart) - Add an optimization with multiple vehicles which start their routes from the same departure and display the solution on the map
* [[VRP] Add Optim With Multiple Vehicles Single Vehicle Constraints](Examples/VRP/AddOptimWithMultiVSingleVConstr) - Add an optimization with multiple vehicles and only one set of vehicle constraints set on all the vehicles and display the solution on the map
* [[VRP] Add Optim With Order Fields](Examples/VRP/AddOptimWithOrderFields) - Add an optimization with orders that have all the fields set and display the solution on the map
* [[VRP] Add Optim With Sequence Pairs](Examples/VRP/AddOptimWithSequencePairs) - Add an optimization with sequence pairs (pick up - delivery order pairs) and display the solution on the map
* [[VRP] Add Optim With Set Matrices](Examples/VRP/AddOptimWithSetMatrices) - Add an optimization with custom matrices and display the solution on the map
* [[VRP] Add Optim With Single Vehicle](Examples/VRP/AddOptimWithSingleVehicle) - Add an optimization with a single vehicle and display the solution on the map
* [[VRP] Add Orders To Optimization](Examples/VRP/AddOrdersToOptimization) - Add a list of orders to an existing optimization
* [[VRP] Add Orders To Optimization With Set Matrices](Examples/VRP/AddOrdersToOptimWithSetMatrices) - Add a list of orders to an existing optimization which has custom matrices
* [[VRP] Add Orders To Route At The End](Examples/VRP/AddOrdersToRouteAtTheEnd) - Add a list of orders at the end of an existing route's orders list
* [[VRP] Add Orders To Route At The Optimal Positions](Examples/VRP/AddOrdersToRouteAtTheOptimPos) - Add a list of orders into an existing route's orders list, at the optimal positions, which are determined by the algorithm
* [[VRP] Add Orders To Route At The Specified Positions](Examples/VRP/AddOrdersToRouteAtTheSpecPos) - The orders are inserted at the specified positions between the route's orders
* [[VRP] Add Orders To Route With Set Matrices](Examples/VRP/AddOrdersToRouteWithSetMatrices) - Add a list of orders into an existing route's orders list, which has the matrices set by the user
* [[VRP] Add Polygon Territory](Examples/VRP/AddPolygonTerritory) - Add a polygon territory; create an optimization with the customer of a territory
* [[VRP] Add Rectangle Territory](Examples/VRP/AddRectangleTerritory) - Add a rectangle territory
* [[VRP] Add Vehicle](Examples/VRP/AddVehicle) - Add a vehicle to the fleet to get a more accurate cost of the route
* [[VRP] Delete Customer](Examples/VRP/DeleteCustomer) - Delete a customer from the agenda
* [[VRP] Delete Optimization](Examples/VRP/DeleteOptimization) - Delete an optimization
* [[VRP] Delete Order From Optimization](Examples/VRP/DeleteOrderFromOptimization) - Delete a order from an optimization
* [[VRP] Delete Order From Route](Examples/VRP/DeleteOrderFromRoute) - Delete a order from a route
* [[VRP] Delete Route](Examples/VRP/DeleteRoute) - Delete a route
* [[VRP] Delete Territory](Examples/VRP/DeleteTerritory) - Delete a territory
* [[VRP] Delete Vehicle](Examples/VRP/DeleteVehicle) - Delete a vehicle
* [[VRP] Generate Territories](Examples/VRP/GenerateTerritories) - Generate a number of polygon territories from a list of coordinates and display them on the map
* [[VRP] Get All Customers](Examples/VRP/GetAllCustomers) - Get the customers that were saved in the agenda
* [[VRP] Get All Optimizations](Examples/VRP/GetAllOptimizations) - Get all the optimizations created
* [[VRP] Get All Routes](Examples/VRP/GetAllRoutes) - Get the routes of each optimization
* [[VRP] Get All Territories](Examples/VRP/GetAllTerritories) - Get all the territories created
* [[VRP] Get All Vehicles](Examples/VRP/GetAllVehicles) - Get all the vehicles of the fleet
* [[VRP] Get Customer](Examples/VRP/GetCustomer) - Get a certain customer from the agenda
* [[VRP] Get Customer History](Examples/VRP/GetCustomerHistory) - Get in which optimizations and routes has the customer made orders
* [[VRP] Get Fuel Prices](Examples/VRP/GetFuelPrices) - Get all the changes of the fuel prices and when they were made
* [[VRP] Get Optimization](Examples/VRP/GetOptimization) - Get a certain optimization
* [[VRP] Get Route](Examples/VRP/GetRoute) - Get a certain route
* [[VRP] Get Solution For Optimization](Examples/VRP/GetSolutionForOptimization) - Get the solution (list of routes) of an optimization
* [[VRP] Get Territory](Examples/VRP/GetTerritory) - Get a certain territory
* [[VRP] Get Vehicle](Examples/VRP/GetVehicle) - Get a certain vehicle
* [[VRP] Merge Routes](Examples/VRP/MergeRoutes) - Merge a list of routes into a new one
* [[VRP] Reoptimize Optimization](Examples/VRP/ReoptimizeOptimization) - Reoptimize an optimization
* [[VRP] Reoptimize Route](Examples/VRP/ReoptimizeRoute) - Reoptimize a route
* [[VRP] Search Customers](Examples/VRP/SearchCustomers) - Retrieve customers from the database that match the specified search filter
* [[VRP] Search Fuel Prices](Examples/VRP/SearchFuelPrices) - Retrieve fuel prices from the database that match the specified search filter
* [[VRP] Search Misc Locations](Examples/VRP/SearchMiscLocations) - Retrieve miscellaneous locations from the database that match the specified search filter
* [[VRP] Search Optimization](Examples/VRP/SearchOptimization) - Retrieve optimizations from the database that match the specified search filter
* [[VRP] Search Orders](Examples/VRP/SearchOrders) - Retrieve orders from the database that match the specified search filter
* [[VRP] Search Routes](Examples/VRP/SearchRoutes) - Retrieve orders from the database that match the specified search filter
* [[VRP] Search Territories](Examples/VRP/SearchTerritories) - Retrieve territories from the database that match the specified search filter
* [[VRP] Search Vehicles](Examples/VRP/SearchVehicles) - Retrieve vehicles from the database that match the specified search filter
* [[VRP] Set Fuel Prices](Examples/VRP/SetFuelPrices) - Set the desired prices for each fuel type
* [[VRP] Unlink Route](Examples/VRP/UnlinkRoute) - Unlink a route from its optimization
* [[VRP] Update Customer](Examples/VRP/UpdateCustomer) - Make changes on a customer from the agenda
* [[VRP] Update Optimization](Examples/VRP/UpdateOptimization) - Make changes on an optimization and return the new solution
* [[VRP] Update Order From Optimization](Examples/VRP/UpdateOrderFromOptimization) - Make changes on a order from an optimization
* [[VRP] Update Route](Examples/VRP/UpdateRoute) - Make changes on a route
* [[VRP] Update Territory](Examples/VRP/UpdateTerritory) - Update the fields of a territory
* [[VRP] Update Vehicle](Examples/VRP/UpdateVehicle) - Update the fields of a vehicle

**Note:** As the very first step, we highly recommend that you get a token from [Magic Lane Portal](https://developer.magiclane.com/api). If no token is set, you can still test your apps, but a watermark will be displayed, and all the online services including mapping, searching, routing, etc. will slow down after a few minutes.

## Build instructions

Step 1. Download the SDK

Step 2. Extract SDK to the predefined folder (SDK)

### Prerequires

Step a. On Debian/Ubuntu, make sure that you have installed the following packages

```$ apt install cmake ninja-build xorg-dev libglu1-mesa-dev libegl1-mesa-dev```

Step b. Set up vcpkg package manager:

The first step is to clone the vcpkg repository from GitHub:

```$ git clone https://github.com/microsoft/vcpkg.git```

SDL2 port of vcpkg package manager under Linux currently requires the following libraries from the system package manager: ```python3-jinja2```

Navigate to the vcpkg directory and execute the bootstrap script:

Linux:

```$ cd vcpkg && ./bootstrap-vcpkg.sh -disableMetrics```

Windows:

```cd vcpkg && bootstrap-vcpkg.bat -disableMetrics```

Configure the ```VCPKG_ROOT``` environment variable:

Linux:
```
$ export VCPKG_ROOT=<path/to/vcpkg>
$ export PATH=$VCPKG_ROOT:$PATH
```

Windows:
```
set VCPKG_ROOT=<C:\path\to\vcpkg>
set PATH=%VCPKG_ROOT%;%PATH%
```

### Configure and build with CMake

#### Linux

> **Note:** The Maps SDK for Linux is available upon request. Please contact Magic Lane at <info@magiclane.com> to obtain the Linux SDK package.

Configure:
```bash
$ cmake --preset=linux-<glfw|sdl>-ninja-<debug|release|relwithdebinfo>
```

Build:
```bash
$ cmake --build --preset=linux-<glfw|sdl>-ninja-<debug|release|relwithdebinfo>-all
```

#### Windows

**Visual Studio (Multi-Config Generator)**

For Visual Studio 2022:
```bash
cmake --preset=windows-<glfw|sdl>-vs2022
cmake --build --preset=windows-<glfw|sdl>-vs2022-<debug|release|relwithdebinfo>-all
```

For Visual Studio 2019 (requires full VS2019 installation):
```bash
cmake --preset=windows-<glfw|sdl>-vs2019
cmake --build --preset=windows-<glfw|sdl>-vs2019-<debug|release|relwithdebinfo>-all
```

For Visual Studio 2019 toolset via VS2022 (requires VS2022 + v142 build tools):
```bash
cmake --preset=windows-<glfw|sdl>-vs2019-toolset
cmake --build --preset=windows-<glfw|sdl>-vs2019-toolset-<debug|release|relwithdebinfo>-all
```

**Ninja (Single-Config Generator)**

Configure:
```bash
cmake --preset=windows-<glfw|sdl>-ninja-<debug|release|relwithdebinfo>
```

Build:
```bash
cmake --build --preset=windows-<glfw|sdl>-ninja-<debug|release|relwithdebinfo>-all
```

#### Configuring API Keys

An API Key is required to unlock the full functionality of these example applications. Follow our [guide](https://developer.magiclane.com/docs/guides/get-started) to generate your API Key.

If no API Key is set, you can still test your apps, but a watermark will be displayed, and all the online services including mapping, searching, routing, etc. will slow down after a few minutes.

Linux:
```bash
export API_TOKEN=<your_token>
```

Windows:
```cmd
set API_TOKEN=<your_token>
```

or directly in the CMake command by providing the option: ```-DAPI_TOKEN="<your_token>"```

The preset is configured to read `API_TOKEN` from the environment variable `$env{API_TOKEN}`.


### Troubleshoot

Windows:

* Make sure that you have installed >= Visual Studio 2019

* If you have VS2022 installed but only VS2019 Build Tools (v142), use the `windows-<glfw|sdl>-vs2019-toolset` presets

* If you have full VS2019 installed, use the `windows-<glfw|sdl>-vs2019` presets

* Preset `windows-glfw-vs2022-address-sanitizer` requires Visual Studio 2022 due to the following issue [link](https://developercommunity.visualstudio.com/t/Using-MD-and-fsanitizeaddress-with-Vi/10462700?q=visual+studio+unable+to+start+native+memory+profiling+session+%22internal+error%22)
  
* May encounter the following warning:

```
The object file directory

    <long/path>

  has XXX characters.  The maximum full path to an object file is 250
  characters (see CMAKE_OBJECT_PATH_MAX).  Object file

    <file>.cpp.obj

  cannot be safely placed under this directory.  The build may not work
  correctly.
```

Follow the [link](https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=powershell) to remove the path length limit and restart PC.


## License

```
Copyright (C) 2021-2026 Magic Lane International B.V.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

A copy of the license is available in the repository's `LICENSE` file.

Contact Magic Lane at <info@magiclane.com> for SDK licensing options.
