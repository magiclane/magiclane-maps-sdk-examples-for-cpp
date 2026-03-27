## Overview

This example app demonstrates the following features:
- Retrieve territories from the database that match the specified search filter.

## How to use the sample

When you run the example app, all territories in the database that contain the `searchTerm` will be retrieved. The `searchTerm` is a string used to search for territories, and it is matched against each field in the territory's object.

## How it works

1. Create two `vrp::Territory` objects with different shapes (circle and polygon) and set the desired fields (name, type, color, etc.).
2. Create a `ProgressListener`, `vrp::Service`, and a `vrp::TerritoryList`.
3. Call the `addTerritory()` method from the `vrp::Service` using the `vrp::Territory` and `ProgressListener`, and wait for the operation to complete.
4. After adding the territories, call the `getTerritories()` method from the `vrp::Service` using the list from step 2.) and the `ProgressListener`.
5. Once the operation is complete, the list from step 2.) will be populated with territories that match the search criteria.
