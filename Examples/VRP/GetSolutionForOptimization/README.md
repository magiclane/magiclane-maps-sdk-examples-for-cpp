## Overview

This example app demonstrates the following features:
- Get the solution (list of routes) of an optimization.

Check how to display on the map in [Add Optimization](../AddOptimization) example or to make changes to it, see the [Update Route](../UpdateRoute) example.

## How to use the sample

When you run the example app, the solution will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::RouteList`.
2. Call the `getSolutionForOptimization()` method from the `vrp::Service` using the list from 1.), the ID of the optimization and the `ProgressListener`.
3. Once the operation completes, the list from 1.) will be populated.
