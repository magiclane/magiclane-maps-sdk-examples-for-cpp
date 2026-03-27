## Overview

This example app demonstrates the following features:
- Update the fields of a vehicle.

If the consumption was changed and the vehicle was used in a route, the next time you access (retrieve) the route, its cost will be updated considering the new consumption of the vehicle.

## How to use the sample

When you run the example app, the changes made on the vehicle will be saved.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Retrieve the vehicle you want to update (see [Get Vehicle](../GetVehicle) example) in a `vrp::Vehicle`.
3. Change the desired fields of the `vrp::Vehicle`.
4. Call the `updateVehicle()` method from the `vrp::Service` using the `vrp::Vehicle` from 2.) and the `ProgressListener` and wait for the operation to be done.
