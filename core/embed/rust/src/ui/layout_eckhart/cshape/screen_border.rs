use crate::ui::{
    display::Color,
    geometry::{Alignment2D, Rect},
    shape::{self, Renderer},
};

use super::super::{
    constant,
    theme::{ICON_BORDER_BL, ICON_BORDER_BR, ICON_BORDER_TOP},
};

/// Custom shape for a full screen border overlay, parameterizable by color.
pub struct ScreenBorder {
    color: Color,
    side_bars: [Rect; 3],
}

impl ScreenBorder {
    pub const WIDTH: i16 = 4;
    pub const TOP_ARC_HEIGHT: i16 = ICON_BORDER_TOP.toif.height();

    pub const fn new(color: Color) -> Self {
        let screen = constant::screen();

        // Bottom bar: from the right edge of bottom-left icon to the left edge of
        // bottom-right icon.
        let bottom_bar_rect = Rect {
            x0: screen.x0 + ICON_BORDER_BL.toif.width(),
            y0: screen.y1 - Self::WIDTH,
            x1: screen.x1 - ICON_BORDER_BR.toif.width(),
            y1: screen.y1,
        };

        // Left bar: from the bottom edge of top-left icon to the top edge of
        // bottom-left icon.
        let left_bar_rect = Rect {
            x0: screen.x0,
            y0: screen.y0 + Self::TOP_ARC_HEIGHT,
            x1: screen.x0 + Self::WIDTH,
            y1: screen.y1 - ICON_BORDER_BL.toif.height(),
        };
        // Right bar: from the bottom edge of top-right icon to the top edge of
        // bottom-right icon.
        let right_bar_rect = Rect {
            x0: screen.x1 - Self::WIDTH,
            y0: screen.y0 + Self::TOP_ARC_HEIGHT,
            x1: screen.x1,
            y1: screen.y1 - ICON_BORDER_BR.toif.height(),
        };
        Self {
            color,
            side_bars: [bottom_bar_rect, left_bar_rect, right_bar_rect],
        }
    }

    pub fn render<'s>(&'s self, alpha: u8, target: &mut impl Renderer<'s>) {
        let screen = constant::screen();

        // Draw the four side bars.
        self.side_bars.iter().for_each(|bar| {
            shape::Bar::new(*bar)
                .with_bg(self.color)
                .with_alpha(alpha)
                .render(target);
        });

        // Draw the four corners.
        [
            (
                screen.top_left(),
                ICON_BORDER_TOP.toif,
                Alignment2D::TOP_LEFT,
            ),
            (
                screen.bottom_left(),
                ICON_BORDER_BL.toif,
                Alignment2D::BOTTOM_LEFT,
            ),
            (
                screen.bottom_right(),
                ICON_BORDER_BR.toif,
                Alignment2D::BOTTOM_RIGHT,
            ),
        ]
        .iter()
        .for_each(|(position, toif, alignment)| {
            shape::ToifImage::new(*position, *toif)
                .with_align(*alignment)
                .with_fg(self.color)
                .with_alpha(alpha)
                .render(target);
        });
    }
}
