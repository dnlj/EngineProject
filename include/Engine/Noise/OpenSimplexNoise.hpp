#pragma once
// TODO: split? inline?
// TODO: move to namespace
// TODO: Cleanup

// Engine
#include <Engine/Noise/Noise.hpp>
#include <Engine/Noise/RangePermutation.hpp>


namespace Engine::Noise {
	// Originally from: https://gist.github.com/KdotJPG/b1270127455a94ac5d19
	class OpenSimplexNoise {
		public:
			// TODO: template params?
			using Int = int32;
			using Float = float32;

			OpenSimplexNoise(int64 seed) : perm{seed} {
			}

			void setSeed(int64 seed) {
				perm = seed;
			}

			// 2D OpenSimplex Noise.
			Float value(Float x, Float y) const {

				// Place input coordinates onto grid.
				Float stretchOffset = (x + y) * STRETCH_CONSTANT_2D;
				Float xs = x + stretchOffset;
				Float ys = y + stretchOffset;

				// Floor to get grid coordinates of rhombus (stretched square) super-cell origin.
				Int xsb = floorTo<Int>(xs);
				Int ysb = floorTo<Int>(ys);

				// Skew out to get actual coordinates of rhombus origin. We'll need these later.
				Float squishOffset = (xsb + ysb) * SQUISH_CONSTANT_2D;
				Float xb = xsb + squishOffset;
				Float yb = ysb + squishOffset;

				// Compute grid coordinates relative to rhombus origin.
				Float xins = xs - xsb;
				Float yins = ys - ysb;

				// Sum those together to get a value that determines which region we're in.
				Float inSum = xins + yins;

				// Positions relative to origin point.
				Float dx0 = x - xb;
				Float dy0 = y - yb;

				// We'll be defining these inside the next block and using them afterwards.
				Float dx_ext, dy_ext;
				Int xsv_ext, ysv_ext;

				Float value = 0;

				// Contribution (1,0)
				Float dx1 = dx0 - 1 - SQUISH_CONSTANT_2D;
				Float dy1 = dy0 - 0 - SQUISH_CONSTANT_2D;
				Float attn1 = 2 - dx1 * dx1 - dy1 * dy1;
				if (attn1 > 0) {
					attn1 *= attn1;
					value += attn1 * attn1 * extrapolate(xsb + 1, ysb + 0, dx1, dy1);
				}

				// Contribution (0,1)
				Float dx2 = dx0 - 0 - SQUISH_CONSTANT_2D;
				Float dy2 = dy0 - 1 - SQUISH_CONSTANT_2D;
				Float attn2 = 2 - dx2 * dx2 - dy2 * dy2;
				if (attn2 > 0) {
					attn2 *= attn2;
					value += attn2 * attn2 * extrapolate(xsb + 0, ysb + 1, dx2, dy2);
				}

				if (inSum <= 1) { // We're inside the triangle (2-Simplex) at (0,0)
					Float zins = 1 - inSum;
					if (zins > xins || zins > yins) { // (0,0) is one of the closest two triangular vertices
						if (xins > yins) {
							xsv_ext = xsb + 1;
							ysv_ext = ysb - 1;
							dx_ext = dx0 - 1;
							dy_ext = dy0 + 1;
						} else {
							xsv_ext = xsb - 1;
							ysv_ext = ysb + 1;
							dx_ext = dx0 + 1;
							dy_ext = dy0 - 1;
						}
					} else { // (1,0) and (0,1) are the closest two vertices.
						xsv_ext = xsb + 1;
						ysv_ext = ysb + 1;
						dx_ext = dx0 - 1 - 2 * SQUISH_CONSTANT_2D;
						dy_ext = dy0 - 1 - 2 * SQUISH_CONSTANT_2D;
					}
				} else { // We're inside the triangle (2-Simplex) at (1,1)
					Float zins = 2 - inSum;
					if (zins < xins || zins < yins) { // (0,0) is one of the closest two triangular vertices
						if (xins > yins) {
							xsv_ext = xsb + 2;
							ysv_ext = ysb + 0;
							dx_ext = dx0 - 2 - 2 * SQUISH_CONSTANT_2D;
							dy_ext = dy0 + 0 - 2 * SQUISH_CONSTANT_2D;
						} else {
							xsv_ext = xsb + 0;
							ysv_ext = ysb + 2;
							dx_ext = dx0 + 0 - 2 * SQUISH_CONSTANT_2D;
							dy_ext = dy0 - 2 - 2 * SQUISH_CONSTANT_2D;
						}
					} else { // (1,0) and (0,1) are the closest two vertices.
						dx_ext = dx0;
						dy_ext = dy0;
						xsv_ext = xsb;
						ysv_ext = ysb;
					}
					xsb += 1;
					ysb += 1;
					dx0 = dx0 - 1 - 2 * SQUISH_CONSTANT_2D;
					dy0 = dy0 - 1 - 2 * SQUISH_CONSTANT_2D;
				}

				// Contribution (0,0) or (1,1)
				Float attn0 = 2 - dx0 * dx0 - dy0 * dy0;
				if (attn0 > 0) {
					attn0 *= attn0;
					value += attn0 * attn0 * extrapolate(xsb, ysb, dx0, dy0);
				}

				// Extra Vertex
				Float attn_ext = 2 - dx_ext * dx_ext - dy_ext * dy_ext;
				if (attn_ext > 0) {
					attn_ext *= attn_ext;
					value += attn_ext * attn_ext * extrapolate(xsv_ext, ysv_ext, dx_ext, dy_ext);
				}

				return value / NORM_CONSTANT_2D;
			}
		private:
			constexpr static Float STRETCH_CONSTANT_2D	= Float(-0.211324865405187); // (1/sqrt(2+1)-1)/2;
			constexpr static Float SQUISH_CONSTANT_2D	= Float(0.366025403784439);  // (sqrt(2+1)-1)/2;
			constexpr static Float STRETCH_CONSTANT_3D	= Float(-1.0) / 6;           // (1/sqrt(3+1)-1)/3;
			constexpr static Float SQUISH_CONSTANT_3D	= Float(1.0) / 3;            // (sqrt(3+1)-1)/3;
			constexpr static Float STRETCH_CONSTANT_4D	= Float(-0.138196601125011); // (1/sqrt(4+1)-1)/4;
			constexpr static Float SQUISH_CONSTANT_4D	= Float(0.309016994374947);  // (sqrt(4+1)-1)/4;
			constexpr static Float NORM_CONSTANT_2D		= Float(47.0);
			constexpr static Float NORM_CONSTANT_3D		= Float(103.0);
			constexpr static Float NORM_CONSTANT_4D		= Float(30.0);
			RangePermutation<256> perm;

			// Gradients for 2D. They approximate the directions to the
			// vertices of an octagon from the center.
			const int8 gradients2D[16] = {
				 5,  2,		 2,  5,
				-5,  2,		-2,  5,
				 5, -2,		 2, -5,
				-5, -2,		-2, -5,
			};

			Float extrapolate(Int xsb, Int ysb, Float dx, Float dy) const {
				int index = perm.value(xsb, ysb) & 0x0E;
				return gradients2D[index] * dx + gradients2D[index + 1] * dy;
			}

	};
}
