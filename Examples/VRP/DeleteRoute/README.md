## Overview

This example app demonstrates the following features:
- Delete a route.

The orders of a deleted route will also be deleted from the optimization to which the route belongs (excepting the orders that are also used in other routes of the same optimization, such as depots).
If the route is the only route of an optimization, then it cannot be deleted, instead delete the optimization (see [Delete Optimization](../DeleteOptimization) example).

## How to use the sample

When you run the example app, a route will be deleted.

## How it works

1. Create a `ProgressListener` and `vrp::Service`.
2. Call the `deleteRoute()` method from the `vrp::Service` using the route's ID and `ProgressListener` and wait for the operation to be done.
