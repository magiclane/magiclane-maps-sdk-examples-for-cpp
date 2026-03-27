## Overview

This example app demonstrates the following features:
- Retrieve routes from the database that match the specified search filter.

## How to use the sample

When you run the example app, all routes in the database that contain the `searchTerm` will be retrieved. The `searchTerm` is a string used to search for routes, and it is matched against the fields in the routes object.

## How it works

1. Create a `vrp::OrderList` and add the orders to it. Each order needs to have a customer set; you can either add a new customer and then set it to the order, or you cand use a previously created customer (see [Get Customer](../GetCustomer) example).
2. Create a `vrp::ConfigurationParameters` and set the desired parameters.
3. Create one `vrp::VehicleConstraints` for each vehicle and set them to a `vrp::VehicleConstraintsList`.
4. Create a `vrp::Optimization`, set the desired fields and the objects created at 1.), 2.) and 3.) to it.
5. Create a `ProgressListener`, `vrp::Service`, `vrp::RouteList`, `vrp::OptimizationList` in which the solution will be returned.
6. Call the `addOptimization()` method from `vrp::Service` using the list from 5.), the `vrp::Optimization` from 4.) and the progress listener.
7. After adding the optimization, call the `getRoutes()` method from the `vrp::Service` using the list from step 5.) and the `ProgressListener`.
8. Once the operation completes, the list from step 5.) will be populated with routes that match the search criteria.
