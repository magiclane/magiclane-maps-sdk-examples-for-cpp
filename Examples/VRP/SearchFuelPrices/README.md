## Overview

This example app demonstrates the following features:
- Retrieve fuel prices from the database that match the specified search filter.

## How to use the sample

When you run the example app, all fuel prices in the database that contain the `searchTerm` will be retrieved. The `searchTerm` is a string used to search for fuel prices, and it is matched against the fields in the fuel price object.

## How it works

1. Initialize `vrp::FuelPricePair` objects for different fuel types (e.g., `Diesel Standard`, `Diesel Premium`) and set their prices.
2. Create a `ProgressListener`, `vrp::Service`, and a `vrp::FuelPricesList`.
3. Add the initialized fuel prices to the database by calling the `addFuelPrices()` method from the `vrp::Service`, and wait for the operation to complete.
4. After adding the fuel prices, call the `getFuelPrices()` method from the `vrp::Service` using the list from step 2.) and the `ProgressListener`.
5. Once the operation completes, the list from step 2.) will be populated with fuel prices that match the search criteria.
