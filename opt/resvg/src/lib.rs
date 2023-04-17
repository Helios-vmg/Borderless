// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/*!
[resvg](https://github.com/RazrFalcon/resvg) is an SVG rendering library.
*/

#![forbid(unsafe_code)]
#![warn(missing_docs)]
#![allow(clippy::field_reassign_with_default)]
#![allow(clippy::identity_op)]
#![allow(clippy::too_many_arguments)]
#![allow(clippy::uninlined_format_args)]
#![allow(clippy::upper_case_acronyms)]
#![allow(clippy::wrong_self_convention)]

pub use tiny_skia;
pub use usvg;

use usvg::NodeExt;

mod clip;
#[cfg(feature = "filter")]
mod filter;
mod image;
mod mask;
mod paint_server;
mod path;
mod render;

pub use crate::render::trim_transparency;

trait OptionLog {
    fn log_none<F: FnOnce()>(self, f: F) -> Self;
}

impl<T> OptionLog for Option<T> {
    #[inline]
    fn log_none<F: FnOnce()>(self, f: F) -> Self {
        self.or_else(|| {
            f();
            None
        })
    }
}

trait ConvTransform {
    fn to_native(&self) -> tiny_skia::Transform;
    fn from_native(_: tiny_skia::Transform) -> Self;
}

impl ConvTransform for usvg::Transform {
    fn to_native(&self) -> tiny_skia::Transform {
        tiny_skia::Transform::from_row(
            self.a as f32,
            self.b as f32,
            self.c as f32,
            self.d as f32,
            self.e as f32,
            self.f as f32,
        )
    }

    fn from_native(ts: tiny_skia::Transform) -> Self {
        Self::new(
            ts.sx as f64,
            ts.ky as f64,
            ts.kx as f64,
            ts.sy as f64,
            ts.tx as f64,
            ts.ty as f64,
        )
    }
}

/// Image fit options.
///
/// All variants will preserve the original aspect.
#[derive(Clone, Copy, PartialEq, Debug)]
pub enum FitTo {
    /// Keep original size.
    Original,
    /// Scale to width.
    Width(u32),
    /// Scale to height.
    Height(u32),
    /// Scale to size.
    Size(u32, u32),
    /// Zoom by factor.
    Zoom(f32),
}

impl FitTo {
    /// Returns `size` preprocessed according to `FitTo`.
    pub fn fit_to(&self, size: usvg::ScreenSize) -> Option<usvg::ScreenSize> {
        let sizef = size.to_size();

        match *self {
            FitTo::Original => Some(size),
            FitTo::Width(w) => {
                let h = (w as f64 * sizef.height() / sizef.width()).ceil();
                usvg::ScreenSize::new(w, h as u32)
            }
            FitTo::Height(h) => {
                let w = (h as f64 * sizef.width() / sizef.height()).ceil();
                usvg::ScreenSize::new(w as u32, h)
            }
            FitTo::Size(w, h) => Some(
                sizef
                    .scale_to(usvg::Size::new(w as f64, h as f64)?)
                    .to_screen_size(),
            ),
            FitTo::Zoom(z) => usvg::Size::new(sizef.width() * z as f64, sizef.height() * z as f64)
                .map(|s| s.to_screen_size()),
        }
    }
}

/// Renders an SVG to pixmap.
///
/// If `fit_to` size differs from `tree.svg_node().size`,
/// SVG would be scaled accordingly.
///
/// `transform` will be used as a root transform.
/// Can be used to position SVG inside the `pixmap`.
pub fn render(
    tree: &usvg::Tree,
    fit_to: FitTo,
    transform: tiny_skia::Transform,
    pixmap: tiny_skia::PixmapMut,
) -> Option<()> {
    let size = fit_to.fit_to(tree.size.to_screen_size())?;
    let mut canvas = render::Canvas::from(pixmap);
    canvas.apply_transform(transform);
    render::render_to_canvas(tree, size, &mut canvas);
    Some(())
}

/// Renders an SVG node to pixmap.
///
/// If `fit_to` differs from `node.calculate_bbox()`,
/// SVG would be scaled accordingly.
///
/// `transform` will be used as a root transform.
/// Can be used to position SVG inside the `pixmap`.
pub fn render_node(
    tree: &usvg::Tree,
    node: &usvg::Node,
    fit_to: FitTo,
    transform: tiny_skia::Transform,
    pixmap: tiny_skia::PixmapMut,
) -> Option<()> {
    let node_bbox = if let Some(bbox) = node.calculate_bbox().and_then(|r| r.to_rect()) {
        bbox
    } else {
        log::warn!("Node '{}' has zero size.", node.id());
        return None;
    };

    let vbox = usvg::ViewBox {
        rect: node_bbox,
        aspect: usvg::AspectRatio::default(),
    };

    let size = fit_to.fit_to(node_bbox.size().to_screen_size())?;
    let mut canvas = render::Canvas::from(pixmap);
    canvas.apply_transform(transform);
    render::render_node_to_canvas(
        tree,
        node,
        vbox,
        size,
        &mut render::RenderState::Ok,
        &mut canvas,
    );
    Some(())
}
