## Overview

This example app demonstrates the following features:
- Reoptimize a route.

Rearranges the orders in a better order of visit, if exists. The latest fuel prices are used to calculate the route's cost (see [Get Fuel Prices](../GetFuelPrices) example).

## How to use the sample

When you run the example app, a new route will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::Route`.
2. Call the `route.reoptimize()` using the `vrp::Route` from 1.) and the `ProgressListener`.
3. Check if the associated request has reached the finished status. Once completed, you can retrieve the updated route by calling the `getRoute()` method, which returns a `vrp::Route` containing the reoptimized route.
