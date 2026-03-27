## Overview

This example app demonstrates the following features:
- Add a vehicle to the fleet to get a more accurate cost of the route

By default the route cost is calculated based on a vehicle that is a car which runs on standard gasoline and has a consumption of 7.5 l/100km.

## How to use the sample

When you run the example app, a vehicle will be saved.

## How it works

1. Create a `vrp::Vehicle` and set the desired fields.
2. Create a `ProgressListener` and `vrp::Service`.
3. Call the `addVehicle()` method from the `vrp::Service` using the `vrp::Vehicle` and `ProgressListener` and wait for the operation to be done.
