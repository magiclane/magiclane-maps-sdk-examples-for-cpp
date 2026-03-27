## Overview

This example app demonstrates the following features:
- Get a certain optimization.

Check how to manipulate it, see the [Update Optimization](../UpdateOptimization) example.

## How to use the sample

When you run the example app, the optimization will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::Optimization`.
2. Call the `getOptimization()` method from the `vrp::Service` using the `vrp::Optimization` from 1.), the ID of the optimization that you want to retrieve and the `ProgressListener`.
3. Once the operation completes, the `vrp::Optimization` from 1.) will be populated.
