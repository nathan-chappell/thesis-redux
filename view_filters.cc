//view_filters.cc

#include "view_filters.h"

void prune_isolated_nodes(View &view) {
  for (auto it = view.viewData.begin(); it != view.viewData.end(); ++it) {
    while (it != view.viewData.end() && it->first->isolated()) {
      view.viewData.erase(it++);
    }
    if (it == view.viewData.end()) {
      break;
    }
  }
}
