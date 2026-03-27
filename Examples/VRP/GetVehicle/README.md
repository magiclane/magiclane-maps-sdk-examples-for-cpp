## Overview

This example app demonstrates the following features:
- Get a certain vehicle.

Check how to update the vehicle in [Update Vehicle](../UpdateVehicle) example.

## How to use the sample

When you run the example app, the vehicle will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::Vehicle`.
2. Call the `getVehicle()` method from the `vrp::Service` using the `vrp::Vehicle` from 1.), the ID of the vehicle that you want to retrieve and the `ProgressListener`.
3. Once the operation completes, the `vrp::Vehicle` from 1.) will be populated.
