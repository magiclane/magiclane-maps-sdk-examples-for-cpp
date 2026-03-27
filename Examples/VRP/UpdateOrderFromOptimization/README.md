## Overview

This example app demonstrates the following features:
- Make changes on a order from an optimization.

The changes will be seen on the optimization's route in which is visited, so consider this method when you want to update a order from a route.

## How to use the sample

When you run the example app, the order changes will be saved and a new optimization is returned.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Retrieve the optimization like in the example `GetOptimization()` in a `vrp::Optimization`.
3. Create a `vrp::Order` and initialize it with the optimization order that you want to update.
4. Make the desired changes on the `vrp::Order` from 3.).
5. Call the `optimization.updateOrder()` method from the `vrp::Optimization` from 2.) using the `vrp::Order` from 3.) and the `ProgressListener`.
6. Once the operation completes, the `vrp::Optimization` from 2.) will contain the new optimization with the order updated.
