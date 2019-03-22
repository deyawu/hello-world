#include "heavy.h"
/*
 * change in the branch successfully
 */
Heavy::Heavy()
{
	for (int i = 0; i < NUM; i++)
	{
		light_cells[i] = 0; 
	}
}
Heavy::~Heavy()
{

}
void Heavy::get_obj_time(DWORD &start, DWORD &end, string c)
{
	int label = hash_s(c) % NUM;
	start = heavy_cells[label].t_start;
	end = heavy_cells[label].t_end;
}
DWORD Heavy::trans_during_time(Transaction *t)    // use obj_name to get start,end time
{
	DWORD start_min=INT_MAX, end_max=INT_MIN,s_tmp,end_tmp;
	vector<string> *p = t->get_obj_obtain();
	for (int i = 0; i < (*p).size(); i++)
	{
		get_obj_time(s_tmp, end_tmp, (*p)[i]);
		start_min = (start_min > s_tmp) ? s_tmp : start_min;
		end_max = (end_max > end_tmp) ? end_max : end_tmp;
	}
	return end_max - start_min;
}

void Heavy::handle_heavy_insert(string &key)
{
	int label = (hash_s(key))%NUM;
	if (heavy_cells[label].is_empty)
	{
		heavy_cells[label] = cell(key, false);   // 没有驱逐发生
	}
	else
	{
		if (key == heavy_cells[label].key)
		{
			heavy_cells[label].vote_pos++;
		}
		else
		{
			heavy_cells[label].vote_neg++;
			if (heavy_cells[label].vote_neg >= heavy_cells[label].vote_pos * Lemda)  //驱逐发生
			{
				handle_light_insert(heavy_cells[label].key, heavy_cells[label].vote_pos); // old key
				heavy_cells[label] = cell(key, true);  // 初始化
			}
			else
			{
				handle_light_insert(key, 1); // new key
			}
		}
	}
}
void Heavy::handle_light_insert(string &c,int value)
{
	int label = hash_s(c) % NUM;
	light_cells[label] += value;
}
