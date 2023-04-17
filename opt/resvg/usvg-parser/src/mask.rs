// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

use std::rc::Rc;

use rosvgtree::{self, AttributeId as AId, ElementId as EId};
use svgtypes::{Length, LengthUnit as Unit};
use usvg_tree::{Group, Mask, Node, NodeKind, Rect, Units};

use crate::rosvgtree_ext::SvgNodeExt2;
use crate::{converter, OptionLog, SvgNodeExt};

pub(crate) fn convert(
    node: rosvgtree::Node,
    state: &converter::State,
    cache: &mut converter::Cache,
) -> Option<Rc<Mask>> {
    // A `mask` attribute must reference a `mask` element.
    if node.tag_name() != Some(EId::Mask) {
        return None;
    }

    // Check if this element was already converted.
    if let Some(mask) = cache.masks.get(node.element_id()) {
        return Some(mask.clone());
    }

    let units = node
        .parse_attribute(AId::MaskUnits)
        .unwrap_or(Units::ObjectBoundingBox);
    let content_units = node
        .parse_attribute(AId::MaskContentUnits)
        .unwrap_or(Units::UserSpaceOnUse);

    let rect = Rect::new(
        node.convert_length(AId::X, units, state, Length::new(-10.0, Unit::Percent)),
        node.convert_length(AId::Y, units, state, Length::new(-10.0, Unit::Percent)),
        node.convert_length(AId::Width, units, state, Length::new(120.0, Unit::Percent)),
        node.convert_length(AId::Height, units, state, Length::new(120.0, Unit::Percent)),
    );
    let rect =
        rect.log_none(|| log::warn!("Mask '{}' has an invalid size. Skipped.", node.element_id()))?;

    // Resolve linked mask.
    let mut mask = None;
    if let Some(link) = node.parse_attribute::<rosvgtree::Node>(AId::Mask) {
        mask = convert(link, state, cache);

        // Linked `mask` must be valid.
        if mask.is_none() {
            return None;
        }
    }

    let mut mask = Mask {
        id: node.element_id().to_string(),
        units,
        content_units,
        rect,
        mask,
        root: Node::new(NodeKind::Group(Group::default())),
    };

    converter::convert_children(node, state, cache, &mut mask.root);

    if mask.root.has_children() {
        let mask = Rc::new(mask);
        cache
            .masks
            .insert(node.element_id().to_string(), mask.clone());
        Some(mask)
    } else {
        // A mask without children is invalid.
        None
    }
}
