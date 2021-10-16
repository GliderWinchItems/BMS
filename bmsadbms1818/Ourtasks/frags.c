			/* Goal: Build a 16b word for the BQ to set discharge FETs. */

			// A cell voltage above 'tmp' are candidates for discharging
			tmp = pbq->cellv_low + pbq->lc.cellbal_del;

			// Limit the number of cells discharging at any given cycle
			pbq->active_ct = 0;    // Count number of cell bits set

			// Prevent adjacent cells from being turned ON
			previous = 0;  // Adjacent active cell restriction

			// Sending this word to BQ will set active cells. 
			pbq->cellbal = 0; // Begin with no cells set

			/* Sort (ascending) on Cell voltages. */
			psort  = &cellv_bal[0];
			qsort(psort, 16, sizeof(struct BQCELLV), compare_v);

			

			

			 // cellbalidx: (0 - 15): Index to start cell balance selections
			for (i = pbq->cellbalidx; i >= 0; i--)
			{
				// Does this cell's voltage qualify for discharging, 
				//   AND is not adjacent to previously selected cell,
				//   AND the number cells selected has not reached max?
			if (( previous == 0) && 
				( pbq->active_ct < pbq->balnumwrk) &&
				( *(pbal + i) > tmp)  )

				{ // Here, yes. Cell is candidate for discharge
					pbq->cellbal |= (1 << i); // Set BQ word bit
					pbq->active_ct += 1;
					previous = 1;
				}
				else
				{ // Here, this cell does not need discharging
					previous = 0;
				}
			}
			// Here, we are the bottom of the stack (index 0) but
			//   not necessarily checked all cells.
			// Go to top of stack and check remaining cells
			i = 15; // Wrap cell index around to top of stack
			previous = 0; // Top of stack has no 'previous' cell
			// j cycles through remaining cells, less one
			for (j = (13-pbq->cellbalidx); j >= 0; j--)
			{
			if ((previous == 0) && 
				(pbq->active_ct < pbq->balnumwrk) &&
				(*(pbal + i) > tmp) )

				{ // Here, yes. Cell is candidate for discharge
					pbq->cellbal |= (1 << i); // Set BQ word bit
					pbq->active_ct += 1;
					previous = 1;
				}
				else
				{ // Here, this cell does not need discharging
					previous = 0;
				}
				i -= 1;
			}

			// Here the last cell in the search usually ends up with the next cell,
			//   which was the first cell, already checked and maybe selected.
			if (i == 15) previous = 0;
			if ((i > 0) &&
				(pbq->active_ct < pbq->balnumwrk) &&
				(previous == 0) && 
				(*(pbal + i) > tmp) &&
				(*(pbal + i - 1) > tmp)  )
			{ // Here, yes. Cell is candidate for discharge
				pbq->cellbal |= (1 << i); // Set BQ word bit
				pbq->active_ct += 1;
			}
			/* Begin next cell search at next cell so that all cells needing
			   discharging have a chance every 16 cell balancing cycles.
			*/
			pbq->cellbalidx += 1; // Start at next cell number next time
			if (pbq->cellbalidx > 15) pbq->cellbalidx = 0;
		}
	}
