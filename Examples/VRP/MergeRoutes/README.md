## Overview

This example app demonstrates the following features:
- Merge a list of routes into a new one.

A new optimization will be created for the merged route. The optimization will have same configuration parameters, vehicle constraints and the rest of the fields as the first route from the list. The merged route will not be optimized.

**Recommendation**: to reoptimize the optimization of the merged route, see [Reoptimize Optimization](../ReoptimizeOptimization) example.

## How to use the sample

When you run the example app, the merged route will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service`, a `LargeIntList` with the route ids to be merged and a `vrp::Route` in which the merged route will be returned.
2. Call the `mergeRoutes()` method from the `vrp::Service` using the `vrp::Route` and list from 1.) and the `ProgressListener`.
3. Once the operation completes, the merged route will be returned in the `vrp::Route` from 1.)

### To display the orders and routes on the map

1. Create a `MapServiceListener`, `OpenGLContext` and `MapView`.
2. Create a `LandmarkList` and a `CoordinatesList` from the route's orders and a `PolygonGeographicArea` from the `CoordinatesList`.
3. Instruct the `MapView` to highlight the `LandmarkList` from 2.) to print the orders.
4. Instruct the `MapView` to center on the `PolygonGeographicArea`.
5. Create a `MarkerCollection` of type `Polyline` and add the route's shape to it.
6. Set the newly created `MarkerCollection` in the markers collections of the map view preferences.
7. Allow the application to run until the map view is fully loaded.
