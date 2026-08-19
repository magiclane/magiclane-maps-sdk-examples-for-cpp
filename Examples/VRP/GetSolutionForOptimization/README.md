## Overview

This example app demonstrates the following features:
- Get the solution (list of routes) of an optimization.

Check how to display on the map in [Add Full Optimization](../AddFullOptimization) example or to make changes to it, see the [Update Route](../UpdateRoute) example.

## How to use the sample

When you run the example app, the solution will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::RouteList`.
2. Retrieve the optimization with `getOptimization()` from the `vrp::Service`, using the optimization id, into a `vrp::Optimization`.
3. Call the `getSolution()` method on the `vrp::Optimization` from 2.) using the `ProgressListener` and the `vrp::RouteList` from 1.).
4. Once the operation completes, the list from 1.) will be populated.
