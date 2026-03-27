## Overview

This example app demonstrates the following features:
- Delete a order from an optimization.

It will also be deleted from the optimization's route.
It cannot be deleted if it is the start order of the optimization or the optimization's routes. If you still want to delete it, first change the optimization's start order (see [Update Optimization](../UpdateOptimization) example) and then you can delete it.

## How to use the sample

When you run the example app, the order will be deleted and the optimization will be updated.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Retrieve the optimization like in the example `GetOptimization()` in a `vrp::Optimization`.
3. Create a `vrp::Order` and initialize it with the order that you want to delete.
4. Call the `deleteOrder()` method from `vrp::Optimization` from 2.) using the `vrp::Order` from 3.) and the `ProgressListener`.
5. Once the operation completes, the new optimization will be returned in the `vrp::Optimization` from 2.).
