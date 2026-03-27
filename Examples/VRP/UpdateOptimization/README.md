## Overview

This example app demonstrates the following features:
- Make changes on an optimization and return the new solution.

Besides the orders, all the other fields can be changed using the method `UpdateOptimization()`; check how to change orders in the [Add Orders To Optimization](../AddOrdersToOptimization), [Delete Order From Optimization](../DeleteOrderFromOptimization) and [Update Order From Optimization](../UpdateOrderFromOptimization) examples.
The optimization can also be reoptimized so a new solution will be returned for the new optimization. If the optimization is not reoptimized, the changed will not be applied over the optimization's routes.

## How to use the sample

When you run the example app, the new solution will be returned.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Retrieve the optimization you want to update (see [Get Optimization](../GetOptimization) example) in a `vrp::Optimization`.
3. Change the desired fields of the `vrp::Optimization`.
4. Create a `vrp::RouteList`, and a `vrp::Request` that will be use for traking the status of the operation.
5. Call the `updateOptimization()` method from the `vrp::Service` using the list from 4.), the `vrp::Optimization` from 2.), the optimization will be reoptimized.
6. Check if the associated request has reached the finished status. Once completed, you can retrieve the optimization results by calling the `getSolution()` method, which returns a `vrp::RouteList` containing the reoptimized routes.
