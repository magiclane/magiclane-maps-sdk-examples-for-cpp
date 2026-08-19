## Overview

This example app demonstrates the following features:
- Show how to define a multi-point **user roadblock** interactively on the map, point by point, with a live road-snapped preview
- Show how to commit the defined points as one persistent user roadblock, list and remove user roadblocks
- Show how the roadblock affects routing: a demo route is recalculated and goes around the blocked road

## How to use the sample

When the example app is run, a demo route is calculated in San Francisco (Embarcadero to Castro) and rendered on the map.

1. Optionally pick the transport mode the roadblock applies to - `Car`, `Lorry`, `Pedestrian` or `Bicycle`.
   The same transport mode is used for the demo route.
2. Click `Define roadblock from points`.
3. Zoom in on a street the route uses and **double click** on it - this places the roadblock start point.
4. Move the mouse: the blue polyline shows the road path from the last confirmed point to the road nearest the cursor.
   **Double click** to confirm that point. Repeat for as many points as needed - the map stays pannable and
   zoomable while the roadblock is being defined (only the double click is intercepted).
5. Click `Finish roadblock`. The points are committed as a single persistent user roadblock, the map centers on it,
   and the demo route is recalculated - it now avoids the blocked road. The panel shows the travel time and distance
   before and after.
6. `remove` drops a single roadblock, `Remove all roadblocks` drops all of them; the route is recalculated either way.

`Cancel` abandons a definition in progress without adding anything.

## How it works

A user roadblock with a path impact zone is defined by a list of coordinates, each of which has to be matched
to a road. Instead of asking the user for exact road coordinates, the traffic service does the matching while
the roadblock is being drawn:

1. `gem::TrafficService().getPersistentRoadblockPathPreview( from, to, transportMode )` takes the last confirmed
   match (`gem::UserRoadblockPathPreviewCoordinate`) and the free cursor position, and returns
   - the coordinates of the road path between them,
   - the road match for the cursor position, to be used as the `from` of the next segment,
   - an error code - `gem::KNoError` means the cursor could be matched to a road.
2. The returned path is drawn with a single **polyline sketch** marker. Sketches are markers with individual
   render settings drawn on top of every other marker collection, which makes them a natural fit for transient
   UI geometry:
   `mapView->preferences().markers().sketches( gem::MT_Polyline ).add( marker, gem::MarkerRenderSettings().setPolylineInnerColor( ... ) )`.
   The confirmed part of the sketch is kept and the tail is replaced on every mouse move
   (`marker.delRange( lastMatchIndex, -1 )` followed by `marker.add( previewCoordinates )`), which produces
   the classic rubber-band drawing effect.
3. On a double click the previewed match - not the raw screen position - is appended to the roadblock
   coordinates list, and the previewed segment becomes part of the confirmed sketch
   (`lastMatchIndex = marker.getCoordinatesCount( 0 )`).
4. Finishing the definition commits the coordinates:
   `gem::TrafficService().addPersistentRoadblock( coords, startUTC, expireUTC, transportMode, id )`.
   With a single coordinate a point roadblock is defined, which may block the matched road both ways; with two
   or more coordinates a path roadblock is defined, blocking the first -> last direction only. The call returns
   the resulting `gem::TrafficEvent` together with an error code - `gem::error::KNoRoute` means the coordinates
   cannot be matched to a road path, `KExist` / `KInUse` that the roadblock or its id already exists.
   The roadblock here is given a one-day lifetime.
5. The preview sketch is removed from the map once the definition ends
   (`sketches( gem::MT_Polyline ).del( sketches( gem::MT_Polyline ).indexOf( marker ) )`); from that point on the
   roadblock itself is rendered by the traffic layer of the map style, so
   `mapView->preferences().setTrafficVisibility( true )` is required for it to be visible.
6. Roadblocks are removed by id with `removePersistentRoadblock( id )` or all at once with
   `removeAllPersistentRoadblocks()`. They are **persistent**: they survive the SDK session, so this example
   clears them on exit.
7. The demo route is calculated with `gem::RoutePreferences().setAvoidTraffic( gem::TA_Roadblocks )`, which makes
   the routing engine avoid roadblock traffic events, user roadblocks included.

Two other details worth noting:

- Double clicks are detected in the touch event listener (a press within 400 ms and 6 pixels of the previous one).
  While a definition is active, that press and its release are **not** forwarded to the map, so the map does not
  zoom in on the click which adds a point. Every other event is forwarded, so panning and zooming keep working. Because the map's own
  gesture detector would still see the first click of each pair, the zoom-in gesture is disabled while a definition
  is in progress with `mapView->preferences().enableTouchGestures( gem::TG_OnDoubleTouch, false )`.
- User roadblock ids have to be unique. Roadblocks are persistent, so an id used by an earlier run of the example is
  still taken and `addPersistentRoadblock` would return `error::KInUse`; the id is therefore derived from the
  roadblock start time.
- User roadblocks are part of the navigation capability, so `gem::Sdk::getCapabilities() & gem::SC_Navigation`
  is checked before a definition is started.
