## Overview

This example app demonstrates the following features:
- Retrieve vehicles from the database that match the specified search filter.

## How to use the sample

When you run the example app, all vehicles in the database that contain the `searchTerm` will be retrieved. The `searchTerm` is a string used to search for vehicles, and it is matched against each field in the vehicle's object.

## How it works

1. Create two `vrp::Vehicle` objects and set the desired fields (name, type, manufacturer, model, etc.).
2. Create a `ProgressListener`, `vrp::Service`, and a `vrp::VehicleList`.
3. Call the `addVehicle()` method from the `vrp::Service` using the `vrp::Vehicle` and `ProgressListener`, and wait for the operation to complete.
4. After adding the vehicles, call the `getVehicles()` method from the `vrp::Service` using the list from step 2.) and the `ProgressListener`.
5. Once the operation is complete, the list from step 2.) will be populated with vehicles that match the search criteria.
