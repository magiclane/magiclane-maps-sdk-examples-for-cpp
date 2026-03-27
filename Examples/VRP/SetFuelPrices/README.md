## Overview

This example app demonstrates the following features:
- Set the desired prices for each fuel type.

Routes cost will be more accurate.

## How to use the sample

When you run the example app, the fuel prices will be updated.

## How it works

1. Create a `vrp::FuelPricePair` for each fuel type which will be updated and add them in a `vrp::FuelPricePairList`.
2. Create a `ProgressListener` and a `vrp::Service`.
3. Call the `setFuelPrices()` method from the `vrp::Service` using the list from 1.) and the `ProgressListener` and wait for the operation to be done.
