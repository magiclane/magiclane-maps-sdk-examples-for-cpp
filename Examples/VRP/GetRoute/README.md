## Overview

This example app demonstrates the following features:
- Get a certain route.

Check how to display on the map in [Add Optimization](../AddOptimization) example or to make changes to it, see the [Update Route](../UpdateRoute) example.

## How to use the sample

When you run the example app, the route will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::Route`.
2. Call the `getRoute()` method from the `vrp::Service` using the `vrp::Route` from 1.), the ID of the route that you want to retrieve and the `ProgressListener`.
3. Once the operation completes, the `vrp::Route` from 1.) will be populated.
