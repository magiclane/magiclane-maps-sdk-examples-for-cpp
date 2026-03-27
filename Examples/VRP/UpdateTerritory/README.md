## Overview

This example app demonstrates the following features:
- Update the fields of a territory.

## How to use the sample

When you run the example app, the changes made on the territory will be saved.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Retrieve the territory you want to update (see [Get Territory](../GetTerritory) example) in a `vrp::Territory`.
3. Change the desired fields of the `vrp::Territory`.
4. Call the `updateTerritory()` method from the `vrp::Service` using the `vrp::Territory` from 2.) and the `ProgressListener` and wait for the operation to be done.
5. The list of customer that are inside the territory was also updated. Access it using the method `territory.getCustomers()`.
