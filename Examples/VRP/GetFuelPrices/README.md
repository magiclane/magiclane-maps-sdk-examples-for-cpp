## Overview

This example app demonstrates the following features:
- Get all the changes of the fuel prices and when they were made.

The first one in the list is the most recent change. All the routes created after this change will use these prices to calculate the cost.

## How to use the sample

When you run the example app, all the changes to the fuel prices will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::FuelPricesList`.
2. Call the `getFuelPrices()` method from the `vrp::Service` using the list from 1.) and the `ProgressListener`.
3. Once the operation completes, the list from 1.) will be populated.
