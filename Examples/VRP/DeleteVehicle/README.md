## Overview

This example app demonstrates the following features:
- Delete a vehicle.

Deleted vehicle will no longer appear in the routes to which the vehicle was assigned; their cost will be re-calculated using the default values (the vehicle's type is a car which runs on standard gasoline and has a consumption of 7.5 l/100km).

## How to use the sample

When you run the example app, a vehicle will be deleted.

## How it works

1. Create a `ProgressListener` and `vrp::Service`.
2. Call the `deleteVehicle()` method from the `vrp::Service` using the vehicle's id and `ProgressListener` and wait for the operation to be done.
