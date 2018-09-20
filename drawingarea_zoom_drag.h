// drawingarea_zoom_drag.h

#include <gtkmm-3.0/gtkmm/drawingarea.h>

#include <complex>
#include <functional>
#include <memory>
//#include <iostream>

using C = std::complex<double>;

using CContext = const Cairo::RefPtr<Cairo::Context> &;

/*
 * DrawingArea_ZoomDrag is a drawing area which handles some zooming and
 * dragging in
 * response to motion events
 */
class DrawingArea_ZoomDrag : public Gtk::DrawingArea {
  Cairo::Matrix m;
  bool drag_;
  C last_pos_;
  std::shared_ptr<C> what_to_drag_;

public:
  /*
   * This call back will perform drawing actions after applying the correct
   * zooming tranformations.
   */
  std::function<bool(CContext)> zoomed_draw;
  /*
   * This call back sets the field what_to_drag.
   */
  std::function<std::shared_ptr<C>(GdkEventButton *)> set_drag;

  DrawingArea_ZoomDrag();
  DrawingArea_ZoomDrag(std::function<bool(CContext)> zoomed_draw,
                       std::function<std::shared_ptr<C>(GdkEventButton *)> set_drag);

  C user_to_image(const C &) const;

protected:
  bool on_button_press_event(GdkEventButton *) override;
  bool on_button_release_event(GdkEventButton *) override;
  bool on_motion_notify_event(GdkEventMotion *) override;
  bool on_draw(CContext) override;
  bool on_scroll_event(GdkEventScroll *e) override;

private:
  void translate_matrix(C);
  void translate_item(C *item, C translate);
};

std::ostream &operator<<(std::ostream &o, const Cairo::Matrix &m);
