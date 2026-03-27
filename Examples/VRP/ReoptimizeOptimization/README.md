## Overview

This example app demonstrates the following features:
- Reoptimize an optimization.

Creates a new and better solution (if exists) for the optimization. The latest fuel prices are used for the new solution (see [Get Fuel Prices](../GetFuelPrices) example).

## How to use the sample

When you run the example app, the optimization will be optimized and a new soltution will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` a `vrp::RouteList` and a `vrp::Request`.
2. Call the `reoptimizeOptimization()` method from the `vrp::Service` using the `vrp::RouteList` from 1.), the ID of the optimization that you will be reoptimized and the `ProgressListener`.
3. Check if the associated request has reached the finished status. Once completed, you can retrieve the optimization results by calling the `getSolution()` method, which returns a `vrp::RouteList` containing the reoptimized routes.