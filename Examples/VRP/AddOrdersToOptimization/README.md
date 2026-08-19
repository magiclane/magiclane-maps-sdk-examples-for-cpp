## Overview

This example app demonstrates the following features:
- Add a list of orders to an existing optimization.

If the optimization is also reoptimized, the added orders will be assigned to the optimization's routes.

## How to use the sample

When you run the example app, in addition to the existing orders, the optimization will also have the added orders

## How it works

1. Create a `vrp::Order` with the desired fields for each order that will be added and insert them in a `vrp::OrderList`.
2. Create a `ProgressListener`, `vrp::Service` and a `vrp::Request` that will be used to track the request status.
3. Retrieve the optimization like in the [Get Optimization](../GetOptimization) example, in a `vrp::Optimization`.
4. Call the `optimization.addOrders()` method from `vrp::Optimization` using the `ProgressListener`, the list from 1.), the `vrp::Request` from 2.) and a boolean to specify if the optimization should be reoptimized (`true` here), in that order.
5. Check if the associated request has reached the finished status. Once completed, the optimization contains the added orders. This sample stops there; to fetch the resulting routes, call `getSolution()` on the `vrp::Optimization` as shown in the [Get Solution For Optimization](../GetSolutionForOptimization) example.
