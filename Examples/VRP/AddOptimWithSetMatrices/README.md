## Overview

This example app demonstrates the following features:
- Add an optimization with custom matrices and display the solution on the map.

Create an optimization where the time and distance matrice values are set by the user.

![](screenshotOrders.png)

![](screenshotSolution.png)

## How to use the sample

When you run the example app, an optimization will be saved, the solution will be returned and showed on map.

## How it works

1. Create a `vrp::OrderList` and add the orders to it. Each order needs to have a customer set; you can either add a new customer and then set it to the order, or you can use a previously created customer (see [Get Customer](../GetCustomer) example).
2. Create a `vrp::ConfigurationParameters` and set the distance unit, the matrices build type to `MBT_Set`, and the matrices to it.
3. Create a `vrp::Optimization` and set the objects created at 1.) and 2.) to it.
4. Create a `ProgressListener`, `vrp::Service`, and a `vrp::Request` that will be used to track the request status.
5. Call the `addOptimization()` method from `vrp::Service` using the request from 4.), the `vrp::Optimization` from 3.), and the progress listener.
6. After adding the optimization, monitor the request until it reaches a finished state. Once completed, retrieve the optimization results by calling the `getSolution()` method, which returns a `vrp::RouteList` containing the generated routes.

### To display the orders and routes on the map

1. Create a `MapServiceListener`, `OpenGLContext` and `MapView`.
2. Create a `LandmarkList`, `CoordinatesList` and `PolygonGeographicArea`.
3. Instruct the `MapView` to highlight the `LandmarkList` from 2.) to print the orders, departures and destinations.
4. Instruct the `MapView` to center on the `PolygonGeographicArea`.
5. Create a `MarkerCollection` of type `Polyline` and add the route's shape to it.
6. Set the newly created `MarkerCollection` in the markers collections of the map view preferences.
7. Allow the application to run until the map view is fully loaded.
