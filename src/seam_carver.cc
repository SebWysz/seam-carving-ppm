#include "seam_carver.hpp"
#include <iostream>

// implement the rest of SeamCarver's functions here
const ImagePPM& SeamCarver::GetImage() const { return image_; }
int SeamCarver::GetHeight() const { return height_; }
int SeamCarver::GetWidth() const { return width_; }

int SeamCarver::GetEnergy(int row, int col) const {
  int prev_row = (row - 1), next_row = (row + 1);
  int prev_col = col - 1, next_col = col + 1;
  if (prev_row == -1) { 
    prev_row = height_ - 1; 
  }
  if (next_row == height_) { 
    next_row = 0;
  }
  if (prev_col == -1) { 
    prev_col = width_ - 1; 
  }
  if (next_col == width_) { 
    next_col = 0; 
  }
  Pixel antes_row = image_.GetPixel(prev_row, col);
  Pixel despues_row = image_.GetPixel(next_row, col);
  Pixel antes_col = image_.GetPixel(row, prev_col);
  Pixel despues_col = image_.GetPixel(row, next_col);
  int delta_row = ((antes_row.GetRed() - despues_row.GetRed()) * (antes_row.GetRed() - despues_row.GetRed())) 
    + ((antes_row.GetGreen() - despues_row.GetGreen()) * (antes_row.GetGreen() - despues_row.GetGreen()))
    + ((antes_row.GetBlue() - despues_row.GetBlue()) * (antes_row.GetBlue() - despues_row.GetBlue()));
  int delta_col = ((antes_col.GetRed() - despues_col.GetRed()) * (antes_col.GetRed() - despues_col.GetRed())) 
    + ((antes_col.GetGreen() - despues_col.GetGreen()) * (antes_col.GetGreen() - despues_col.GetGreen()))
    + ((antes_col.GetBlue() - despues_col.GetBlue()) * (antes_col.GetBlue() - despues_col.GetBlue()));
  
  return delta_col + delta_row;
}

int* SeamCarver::GetHorizontalSeam() const {
  int** horiz_memo = InitHorizMemo();
  int* seam_to_return = new int[width_];
  int min_idx = 0, min_energy = horiz_memo[0][0];
  for (int row = 1; row < height_; row++) {
    if (horiz_memo[row][0] < min_energy) {
      min_energy = horiz_memo[row][0];
      min_idx = row;
    }
  }
  seam_to_return[0] = min_idx;
  min_energy -= GetEnergy(min_idx, 0);
  for (int col = 1; col < width_; col++) {
    if (min_energy == horiz_memo[seam_to_return[col - 1]][col]) {
      seam_to_return[col] = seam_to_return[col - 1];
    } else if (seam_to_return[col - 1] != 0 && min_energy == horiz_memo[seam_to_return[col - 1] - 1][col]) {
      seam_to_return[col] = seam_to_return[col - 1] - 1;
    } else if (seam_to_return[col - 1] != height_ - 1 && min_energy == horiz_memo[seam_to_return[col - 1] + 1][col]) {
      seam_to_return[col] = seam_to_return[col - 1] + 1;
    }
    min_energy -= GetEnergy(seam_to_return[col], col);
  }
  for (int row = 0; row < height_; row++) {
    delete[] horiz_memo[row];
  }
  delete[] horiz_memo;
  horiz_memo = nullptr;
  return seam_to_return;
}

int* SeamCarver::GetVerticalSeam() const {
  int** vert_memo = InitVertMemo();
  int* seam_to_return = new int[height_];
  int min_idx = 0, min_energy = vert_memo[0][0];
  for (int col = 1; col < width_; col++) {
    if (vert_memo[0][col] < min_energy) {
      min_energy = vert_memo[0][col];
      min_idx = col;
    }
  }

  seam_to_return[0] = min_idx;
  min_energy -= GetEnergy(0, min_idx);
  for (int row = 1; row < height_; row++) {
    if (min_energy == vert_memo[row][seam_to_return[row - 1]]) {
      seam_to_return[row] = seam_to_return[row - 1];
    } else if (seam_to_return[row - 1] != 0 && min_energy == vert_memo[row][seam_to_return[row - 1] - 1]) {
      seam_to_return[row] = seam_to_return[row - 1] - 1;
    } else if (seam_to_return[row - 1] != width_ - 1 && min_energy == vert_memo[row][seam_to_return[row - 1] + 1]) {
      seam_to_return[row] = seam_to_return[row - 1] + 1;
    }
    min_energy -= GetEnergy(row, seam_to_return[row]);
  }
  for (int row = 0; row < height_; row++) {
    delete[] vert_memo[row];
  }
  delete[] vert_memo;
  vert_memo = nullptr;
  return seam_to_return;
}

void SeamCarver::RemoveHorizontalSeam() {
  image_.CarveHorizSeam(GetHorizontalSeam());
  height_ = image_.GetHeight();
}

void SeamCarver::RemoveVerticalSeam() {
  image_.CarveVertSeam(GetVerticalSeam());
  width_ = image_.GetWidth();
}

int** SeamCarver::InitHorizMemo() const {
  int** horiz_memo = new int*[height_];
  for (int row = 0; row < height_; row++) {
    horiz_memo[row] = new int[width_];
    for (int col = 0; col < width_; col++) {
      horiz_memo[row][col] = -1;
    }
  }

  for (int row = 0; row < height_; row++) {
    horiz_memo[row][width_ - 1] = GetEnergy(row, width_ - 1);
  }
  for (int col = width_ - 2; col >= 0; col--) {
    for (int row = 0; row < height_; row++) {
      int next_step = horiz_memo[row][col + 1];
      if (row != 0 && horiz_memo[row - 1][col + 1] < next_step) {
        next_step = horiz_memo[row - 1][col + 1];
      } 
      if (row != height_ - 1 && horiz_memo[row + 1][col + 1] < next_step) {
        next_step = horiz_memo[row + 1][col + 1];
      }
      horiz_memo[row][col] = next_step + GetEnergy(row, col);
    }
  }
  return horiz_memo;
}

int** SeamCarver::InitVertMemo() const {
  int** vert_memo = new int*[height_];
  for (int row = 0; row < height_; row++) {
    vert_memo[row] = new int[width_];
    for (int col = 0; col < width_; col++) {
      vert_memo[row][col] = -1;
    }
  }

  for (int col = 0; col < width_; col++) {
    vert_memo[height_ - 1][col] = GetEnergy(height_ - 1, col);
  }
  for (int row = height_ - 2; row >= 0; row--) {
    for (int col = 0; col < width_; col++) {
      int next_step = vert_memo[row + 1][col]; 
      if (col != 0 && vert_memo[row + 1][col - 1] < next_step) {
        next_step = vert_memo[row + 1][col - 1];
      }
      if (col != width_ - 1 && vert_memo[row + 1][col + 1] < next_step) {
        next_step = vert_memo[row + 1][col + 1];
      }
      vert_memo[row][col] = next_step + GetEnergy(row, col);
    }
  }
  return vert_memo;
}

// given functions below, DO NOT MODIFY

SeamCarver::SeamCarver(const ImagePPM& image): image_(image) {
  height_ = image.GetHeight();
  width_ = image.GetWidth();
}

void SeamCarver::SetImage(const ImagePPM& image) {
  image_ = image;
  width_ = image.GetWidth();
  height_ = image.GetHeight();
}
