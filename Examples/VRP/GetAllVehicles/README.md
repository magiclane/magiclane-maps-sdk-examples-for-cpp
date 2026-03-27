## Overview

This example app demonstrates the following features:
- Get all the vehicles of the fleet.

## How to use the sample

When you run the example app, all the vehicles will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::VehicleList`.
2. Call the `getAllVehicles()` method from the `vrp::Service` using the list from 1.) and the `ProgressListener`.
3. Once the operation completes, the list from 1.) will be populated.
