// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

use rosvgtree::{self, AttributeId as AId};
use svgtypes::{Length, LengthUnit as Unit};
use usvg_tree::Units;

use crate::converter;
use crate::rosvgtree_ext::SvgNodeExt2;

#[inline(never)]
pub(crate) fn convert_length(
    length: Length,
    node: rosvgtree::Node,
    aid: AId,
    object_units: Units,
    state: &converter::State,
) -> f64 {
    let dpi = state.opt.dpi;
    let n = length.number;
    match length.unit {
        Unit::None | Unit::Px => n,
        Unit::Em => n * resolve_font_size(node, state),
        Unit::Ex => n * resolve_font_size(node, state) / 2.0,
        Unit::In => n * dpi,
        Unit::Cm => n * dpi / 2.54,
        Unit::Mm => n * dpi / 25.4,
        Unit::Pt => n * dpi / 72.0,
        Unit::Pc => n * dpi / 6.0,
        Unit::Percent => {
            if object_units == Units::ObjectBoundingBox {
                length.number / 100.0
            } else {
                let view_box = state.view_box;

                match aid {
                    AId::Cx
                    | AId::Dx
                    | AId::Fx
                    | AId::MarkerWidth
                    | AId::RefX
                    | AId::Rx
                    | AId::Width
                    | AId::X
                    | AId::X1
                    | AId::X2 => convert_percent(length, view_box.width()),
                    AId::Cy
                    | AId::Dy
                    | AId::Fy
                    | AId::Height
                    | AId::MarkerHeight
                    | AId::RefY
                    | AId::Ry
                    | AId::Y
                    | AId::Y1
                    | AId::Y2 => convert_percent(length, view_box.height()),
                    _ => {
                        let mut vb_len = view_box.width().powi(2) + view_box.height().powi(2);
                        vb_len = (vb_len / 2.0).sqrt();
                        convert_percent(length, vb_len)
                    }
                }
            }
        }
    }
}

#[inline(never)]
pub(crate) fn convert_list(
    node: rosvgtree::Node,
    aid: AId,
    state: &converter::State,
) -> Option<Vec<f64>> {
    if let Some(text) = node.attribute(aid) {
        let mut num_list = Vec::new();
        for length in svgtypes::LengthListParser::from(text).flatten() {
            num_list.push(convert_length(
                length,
                node,
                aid,
                Units::UserSpaceOnUse,
                state,
            ));
        }

        Some(num_list)
    } else {
        None
    }
}

fn convert_percent(length: Length, base: f64) -> f64 {
    base * length.number / 100.0
}

#[inline(never)]
pub(crate) fn resolve_font_size(node: rosvgtree::Node, state: &converter::State) -> f64 {
    let nodes: Vec<_> = node.ancestors().collect();
    let mut font_size = state.opt.font_size;
    for n in nodes.iter().rev().skip(1) {
        // skip Root
        if let Some(length) = n.parse_attribute::<Length>(AId::FontSize) {
            let dpi = state.opt.dpi;
            let n = length.number;
            font_size = match length.unit {
                Unit::None | Unit::Px => n,
                Unit::Em => n * font_size,
                Unit::Ex => n * font_size / 2.0,
                Unit::In => n * dpi,
                Unit::Cm => n * dpi / 2.54,
                Unit::Mm => n * dpi / 25.4,
                Unit::Pt => n * dpi / 72.0,
                Unit::Pc => n * dpi / 6.0,
                Unit::Percent => {
                    // If `font-size` has percent units that it's value
                    // is relative to the parent node `font-size`.
                    length.number * font_size * 0.01
                }
            }
        } else if let Some(name) = n.attribute(AId::FontSize) {
            font_size = convert_named_font_size(name, font_size);
        }
    }

    font_size
}

fn convert_named_font_size(name: &str, parent_font_size: f64) -> f64 {
    let factor = match name {
        "xx-small" => -3,
        "x-small" => -2,
        "small" => -1,
        "medium" => 0,
        "large" => 1,
        "x-large" => 2,
        "xx-large" => 3,
        "smaller" => -1,
        "larger" => 1,
        _ => {
            log::warn!("Invalid 'font-size' value: '{}'.", name);
            0
        }
    };

    // 'On a computer screen a scaling factor of 1.2 is suggested between adjacent indexes.'
    parent_font_size * 1.2f64.powi(factor)
}
