## Overview

This example app demonstrates the following features:
- Get a certain territory.

Check how to update the vehicle in [Update Vehicle](../UpdateVehicle) example, to display it on the map or to have access to the customers inside it in order to create an optimization with them in [Add Polygon Territory](../AddPolygonTerritory) example.

## How to use the sample

When you run the example app, the territory will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::Territory`.
2. Call the `getTerritory()` method from the `vrp::Service` using the `vrp::Territory` from 1.), the ID of the territory that you want to retrieve and the `ProgressListener`.
3. Once the operation completes, the `vrp::Territory` from 1.) will be populated.
