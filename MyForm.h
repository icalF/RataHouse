#pragma once

#include <cmath>
#include "Interval.h"
#include "Device.h"
#include <Windows.h>
#include <assert.h>
#include <algorithm>
#include <cstdio>
#include <vector>

char tmp[1024];
int totalSlot, ninterval, ProLevel, powerLimit, ndevice;
std::vector<Interval> intervals;
std::vector< std::vector<int> > proCost;
std::vector<int> ProLimit;
std::vector<Device> devices;
std::vector<double> mean;
char* input;

using namespace System::Runtime::InteropServices;

// inputing all variables from chosen input file
void FileInput()
{
	FILE* fi = fopen(input, "r");

	fscanf(fi, "%d", &totalSlot);
	fscanf(fi, "%d", &ninterval);
	fscanf(fi, "%d", &ProLevel);
	fscanf(fi, "%d", &powerLimit);

	intervals.resize(ninterval + 2);
	ProLimit.resize(ProLevel);
	proCost.resize(ProLevel);

	for (int i = 0; i < ProLevel; ++i)
	{
		proCost[i].resize(ninterval);
		for (int j = 0; j < ninterval; ++j)
		{
			fscanf(fi, "%d", &intervals[j].begin);
			fscanf(fi, "%d", &intervals[j].end);
			fscanf(fi, "%s", tmp);

			if (!j)
				ProLimit[i] = (tmp[0] != '>' ? atoi(tmp) : 999999);

			fscanf(fi, "%d", &proCost[i][j]);
		}
	}

	fscanf(fi, "%d", &ndevice);
	devices.resize(ndevice);
	for (Device& d : devices)
	{
		fscanf(fi, "%s", tmp);
		d.name = string(tmp);
		fscanf(fi, "%d", &d.power);
		fscanf(fi, "%d", &d.slot);
		fscanf(fi, "%d", &d.permittedRange.begin);
		d.permittedRange.begin *= 2;
		fscanf(fi, "%d", &d.permittedRange.end);
		d.permittedRange.end *= 2;
		fscanf(fi, "%s", tmp);
		d.wajib = (string(tmp) == string("wajib"));
		fscanf(fi, "%d", &d.nyala);
	}

	fclose(fi);
}

// find cost of using certain slot
inline int slotCost(int curSlot, int power)
{
	int curIntv =
		(upper_bound(intervals.begin(), intervals.end(),
			Interval(curSlot, curSlot))
			- intervals.begin());
	return *upper_bound(proCost[curIntv].begin(), proCost[curIntv].end(), power);
}

// compute total cost of current configuration
// return -1 if answer doesnt exist
// and return actual total cost
int GetCost()
{
	int power[50];
	int result = 0;

	for (Device dev : devices)
	{
		if (!dev.IsPermitted()) return -1;

		for (Interval& rng : dev.assignedRange)
		{
			power[rng.begin] += dev.power;
			power[rng.end] -= dev.power;
		}
	}

	if (power[0] > powerLimit) return -1;
	for (int i = 1; i < 48; ++i)
	{
		power[i] += power[i - 1];
		if (power[i] > powerLimit) return -1;
	}

	for (int i = 0; i < 48; ++i)
	{
		result += slotCost(i, power[i]);
	}

	return result;
}

// set posisi tiap range
bool set(Device& d)
{
	if (d.permittedRange.end - d.permittedRange.begin < d.slot * d.nyala)
		return false;

	for (int i = 0; i < d.nyala; ++i)
	{
		d.assignedRange.push_back(
			Interval(d.permittedRange.begin + i * d.slot,
				d.permittedRange.begin + (i + 1) * d.slot)
		);
	}

	bool result = true;
	int ujung = d.permittedRange.end;
	for (int i = d.assignedRange.size() - 1; i >= 0; --i)
	{
		double const_sekre = 0.37;	//Secretary Theorem
		Interval& curRange = d.assignedRange[i];
		int const_limit = (ujung - curRange.end) * const_sekre;
		int mini_cost;
		bool vavalita;

		mini_cost = GetCost();
		for (int j = 0; j<const_limit; j++) {
			curRange++;
			int temp_cost = GetCost(); //PERLU DIGANTI, FUNGSI GLOBAL, GETCOST DARI SEMUA KONF.
			if (temp_cost> -1) {
				if (temp_cost<mini_cost) {
					mini_cost = temp_cost;
				}
			}
		}
		int j = const_limit;
		bool ketemu = false;
		while (!ketemu && j<ujung) {
			curRange++;
			int temp_cost = GetCost(); //PERLU DIGANTI, FUNGSI GLOBAL, GETCOST DARI SEMUA KONF.
			if (temp_cost> -1) {
				if (temp_cost <= mini_cost) {
					ketemu = true;
					mini_cost = temp_cost;
				}
			}
			if (j == ujung - 1 && !ketemu) {
				ketemu = true;
				mini_cost = temp_cost;
			}
			j++;
		}
		if (mini_cost == -1) {
			result = false;
		}
		else
		{
			result = true;
		}
	}
}

// calculate mean of progressive graphics
// heuristic approach
void calculateMean()
{
	std::vector< std::vector<int> > vero;
	vero.resize(ProLevel);

	for (int i = 0; i < ProLevel; i++)
	{
		vero[i].resize(50);
		for (int j = 0; j < ninterval; j++)
		{
			Interval& tmp = intervals[j];

			vero[i][tmp.begin] += proCost[i][j];
			vero[i][tmp.end] -= proCost[i][j];
		}
	}

	for (int i = 0; i < ProLevel; i++)
	{
		for (int j = 1; j < 48; j++)
		{
			vero[i][j] += vero[i][j - 1];
		}
	}

	mean.resize(49);
	for (int i = 0; i < 48; i++)
	{
		double sum = 0;

		for (int j = 0; j < ProLevel; j++)
		{
			sum += vero[j][i];
		}

		sum /= ProLevel;
		mean[i] = sum;
	}
}

// sorting devices according to priority value
// as explained in docs
void sortDevices()
{
	calculateMean();

	for (Device& dev : devices)
	{
		for (Interval& rng : dev.assignedRange)
			for (int slotUsed = rng.begin;
				slotUsed < rng.end; ++slotUsed)
		{
			dev.value += mean[slotUsed] * slotCost(slotUsed, dev.power);
		}
		dev.value *= dev.power * dev.nyala;
		dev.value /= dev.slot;
	}

	sort(devices.begin(), devices.end());
}

// main solver :
// returning answer availability (exist / not)
bool Solve()
{
	FileInput();
	calculateMean();
	sortDevices();

	int maxSkip = max(1, ndevice / 20);
	int skip = 0;
	for (Device& d : devices)
	{
		if (skip == maxSkip)
		{
			return false;
		}
		else if (set(d) == false)
		{
			skip++;
			continue;
		}
		else
		{
			skip = 0;
		}
	}

	return true;
}

namespace RataHouse
{

	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

	public:	 
		char* pathfile;
		bool exe = false;

	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::DataGridView^  dataGridView1;

	private: System::Windows::Forms::Label^  label1;


	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column49;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column50;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column1;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column2;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column3;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column4;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column5;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column6;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column7;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column8;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column9;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column10;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column11;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column12;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column13;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column14;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column15;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column16;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column17;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column18;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column19;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column20;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column21;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column22;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column23;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column24;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column25;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column26;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column27;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column28;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column29;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column30;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column31;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column32;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column33;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column34;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column35;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column36;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column37;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column38;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column39;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column40;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column41;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column42;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column43;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column44;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column45;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column46;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column47;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column48;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column51;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column52;

	public: bool eme = false;

	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
	private: System::Windows::Forms::Button^  button2;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->dataGridView1 = (gcnew System::Windows::Forms::DataGridView());
			this->Column49 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column50 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column1 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column2 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column3 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column4 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column5 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column6 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column7 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column8 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column9 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column10 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column11 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column12 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column13 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column14 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column15 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column16 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column17 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column18 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column19 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column20 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column21 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column22 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column23 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column24 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column25 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column26 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column27 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column28 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column29 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column30 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column31 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column32 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column33 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column34 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column35 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column36 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column37 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column38 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column39 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column40 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column41 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column42 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column43 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column44 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column45 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column46 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column47 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column48 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column51 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column52 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView1))->BeginInit();
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->BackColor = System::Drawing::Color::Navy;
			this->button1->ForeColor = System::Drawing::SystemColors::ControlLightLight;
			this->button1->Location = System::Drawing::Point(36, 12);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(99, 34);
			this->button1->TabIndex = 4;
			this->button1->Text = L"Open FIle";
			this->button1->UseVisualStyleBackColor = false;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
			// 
			// openFileDialog1
			// 
			this->openFileDialog1->FileName = L"openFileDialog1";
			// 
			// button2
			// 
			this->button2->BackColor = System::Drawing::Color::Indigo;
			this->button2->ForeColor = System::Drawing::SystemColors::ButtonHighlight;
			this->button2->Location = System::Drawing::Point(776, 12);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(100, 34);
			this->button2->TabIndex = 0;
			this->button2->UseVisualStyleBackColor = false;
			this->button2->Visible = false;
			this->button2->Click += gcnew System::EventHandler(this, &MyForm::button2_Click);
			// 
			// label2
			// 
			this->label2->BackColor = System::Drawing::SystemColors::ButtonFace;
			this->label2->Location = System::Drawing::Point(0, 0);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(100, 23);
			this->label2->TabIndex = 0;
			this->label2->Visible = false;
			// 
			// dataGridView1
			// 
			this->dataGridView1->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridView1->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(52) {
				this->Column49,
					this->Column50, this->Column1, this->Column2, this->Column3, this->Column4, this->Column5, this->Column6, this->Column7, this->Column8,
					this->Column9, this->Column10, this->Column11, this->Column12, this->Column13, this->Column14, this->Column15, this->Column16,
					this->Column17, this->Column18, this->Column19, this->Column20, this->Column21, this->Column22, this->Column23, this->Column24,
					this->Column25, this->Column26, this->Column27, this->Column28, this->Column29, this->Column30, this->Column31, this->Column32,
					this->Column33, this->Column34, this->Column35, this->Column36, this->Column37, this->Column38, this->Column39, this->Column40,
					this->Column41, this->Column42, this->Column43, this->Column44, this->Column45, this->Column46, this->Column47, this->Column48,
					this->Column51, this->Column52
			});
			this->dataGridView1->Location = System::Drawing::Point(12, 75);
			this->dataGridView1->Name = L"dataGridView1";
			this->dataGridView1->RowTemplate->Resizable = System::Windows::Forms::DataGridViewTriState::True;
			this->dataGridView1->Size = System::Drawing::Size(899, 246);
			this->dataGridView1->TabIndex = 5;
			// 
			// Column49
			// 
			this->Column49->HeaderText = L"Name";
			this->Column49->Name = L"Column49";
			// 
			// Column50
			// 
			this->Column50->HeaderText = L"Wajib/tidak";
			this->Column50->Name = L"Column50";
			// 
			// Column1
			// 
			this->Column1->HeaderText = L"1";
			this->Column1->Name = L"Column1";
			this->Column1->Width = 30;
			// 
			// Column2
			// 
			this->Column2->HeaderText = L"2";
			this->Column2->Name = L"Column2";
			this->Column2->Width = 30;
			// 
			// Column3
			// 
			this->Column3->HeaderText = L"3";
			this->Column3->Name = L"Column3";
			this->Column3->Width = 30;
			// 
			// Column4
			// 
			this->Column4->HeaderText = L"4";
			this->Column4->Name = L"Column4";
			this->Column4->Width = 30;
			// 
			// Column5
			// 
			this->Column5->HeaderText = L"5";
			this->Column5->Name = L"Column5";
			this->Column5->Width = 30;
			// 
			// Column6
			// 
			this->Column6->HeaderText = L"6";
			this->Column6->Name = L"Column6";
			this->Column6->Width = 30;
			// 
			// Column7
			// 
			this->Column7->HeaderText = L"7";
			this->Column7->Name = L"Column7";
			this->Column7->Width = 30;
			// 
			// Column8
			// 
			this->Column8->HeaderText = L"8";
			this->Column8->Name = L"Column8";
			this->Column8->Width = 30;
			// 
			// Column9
			// 
			this->Column9->HeaderText = L"9";
			this->Column9->Name = L"Column9";
			this->Column9->Width = 30;
			// 
			// Column10
			// 
			this->Column10->HeaderText = L"10";
			this->Column10->Name = L"Column10";
			this->Column10->Width = 30;
			// 
			// Column11
			// 
			this->Column11->HeaderText = L"11";
			this->Column11->Name = L"Column11";
			this->Column11->Width = 30;
			// 
			// Column12
			// 
			this->Column12->HeaderText = L"12";
			this->Column12->Name = L"Column12";
			this->Column12->Width = 30;
			// 
			// Column13
			// 
			this->Column13->HeaderText = L"13";
			this->Column13->Name = L"Column13";
			this->Column13->Width = 30;
			// 
			// Column14
			// 
			this->Column14->HeaderText = L"14";
			this->Column14->Name = L"Column14";
			this->Column14->Width = 30;
			// 
			// Column15
			// 
			this->Column15->HeaderText = L"15";
			this->Column15->Name = L"Column15";
			this->Column15->Width = 30;
			// 
			// Column16
			// 
			this->Column16->HeaderText = L"16";
			this->Column16->Name = L"Column16";
			this->Column16->Width = 30;
			// 
			// Column17
			// 
			this->Column17->HeaderText = L"17";
			this->Column17->Name = L"Column17";
			this->Column17->Width = 30;
			// 
			// Column18
			// 
			this->Column18->HeaderText = L"18";
			this->Column18->Name = L"Column18";
			this->Column18->Width = 30;
			// 
			// Column19
			// 
			this->Column19->HeaderText = L"19";
			this->Column19->Name = L"Column19";
			this->Column19->Width = 30;
			// 
			// Column20
			// 
			this->Column20->HeaderText = L"20";
			this->Column20->Name = L"Column20";
			this->Column20->Width = 30;
			// 
			// Column21
			// 
			this->Column21->HeaderText = L"21";
			this->Column21->Name = L"Column21";
			this->Column21->Width = 30;
			// 
			// Column22
			// 
			this->Column22->HeaderText = L"22";
			this->Column22->Name = L"Column22";
			this->Column22->Width = 30;
			// 
			// Column23
			// 
			this->Column23->HeaderText = L"23";
			this->Column23->Name = L"Column23";
			this->Column23->Width = 30;
			// 
			// Column24
			// 
			this->Column24->HeaderText = L"24";
			this->Column24->Name = L"Column24";
			this->Column24->Width = 30;
			// 
			// Column25
			// 
			this->Column25->HeaderText = L"25";
			this->Column25->Name = L"Column25";
			this->Column25->Width = 30;
			// 
			// Column26
			// 
			this->Column26->HeaderText = L"26";
			this->Column26->Name = L"Column26";
			this->Column26->Width = 30;
			// 
			// Column27
			// 
			this->Column27->HeaderText = L"27";
			this->Column27->Name = L"Column27";
			this->Column27->Width = 30;
			// 
			// Column28
			// 
			this->Column28->HeaderText = L"28";
			this->Column28->Name = L"Column28";
			this->Column28->Width = 30;
			// 
			// Column29
			// 
			this->Column29->HeaderText = L"29";
			this->Column29->Name = L"Column29";
			this->Column29->Width = 30;
			// 
			// Column30
			// 
			this->Column30->HeaderText = L"30";
			this->Column30->Name = L"Column30";
			this->Column30->Width = 30;
			// 
			// Column31
			// 
			this->Column31->HeaderText = L"31";
			this->Column31->Name = L"Column31";
			this->Column31->Width = 30;
			// 
			// Column32
			// 
			this->Column32->HeaderText = L"32";
			this->Column32->Name = L"Column32";
			this->Column32->Width = 30;
			// 
			// Column33
			// 
			this->Column33->HeaderText = L"33";
			this->Column33->Name = L"Column33";
			this->Column33->Width = 30;
			// 
			// Column34
			// 
			this->Column34->HeaderText = L"34";
			this->Column34->Name = L"Column34";
			this->Column34->Width = 30;
			// 
			// Column35
			// 
			this->Column35->HeaderText = L"35";
			this->Column35->Name = L"Column35";
			this->Column35->Width = 30;
			// 
			// Column36
			// 
			this->Column36->HeaderText = L"36";
			this->Column36->Name = L"Column36";
			this->Column36->Width = 30;
			// 
			// Column37
			// 
			this->Column37->HeaderText = L"37";
			this->Column37->Name = L"Column37";
			this->Column37->Width = 30;
			// 
			// Column38
			// 
			this->Column38->HeaderText = L"38";
			this->Column38->Name = L"Column38";
			this->Column38->Width = 30;
			// 
			// Column39
			// 
			this->Column39->HeaderText = L"39";
			this->Column39->Name = L"Column39";
			this->Column39->Width = 30;
			// 
			// Column40
			// 
			this->Column40->HeaderText = L"40";
			this->Column40->Name = L"Column40";
			this->Column40->Width = 30;
			// 
			// Column41
			// 
			this->Column41->HeaderText = L"41";
			this->Column41->Name = L"Column41";
			this->Column41->Width = 30;
			// 
			// Column42
			// 
			this->Column42->HeaderText = L"42";
			this->Column42->Name = L"Column42";
			this->Column42->Width = 30;
			// 
			// Column43
			// 
			this->Column43->HeaderText = L"43";
			this->Column43->Name = L"Column43";
			this->Column43->Width = 30;
			// 
			// Column44
			// 
			this->Column44->HeaderText = L"44";
			this->Column44->Name = L"Column44";
			this->Column44->Width = 30;
			// 
			// Column45
			// 
			this->Column45->HeaderText = L"45";
			this->Column45->Name = L"Column45";
			this->Column45->Width = 30;
			// 
			// Column46
			// 
			this->Column46->HeaderText = L"46";
			this->Column46->Name = L"Column46";
			this->Column46->Width = 30;
			// 
			// Column47
			// 
			this->Column47->HeaderText = L"47";
			this->Column47->Name = L"Column47";
			this->Column47->Width = 30;
			// 
			// Column48
			// 
			this->Column48->HeaderText = L"48";
			this->Column48->Name = L"Column48";
			this->Column48->Width = 30;
			// 
			// Column51
			// 
			this->Column51->HeaderText = L"49";
			this->Column51->Name = L"Column51";
			// 
			// Column52
			// 
			this->Column52->HeaderText = L"50";
			this->Column52->Name = L"Column52";
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->ForeColor = System::Drawing::SystemColors::ButtonHighlight;
			this->label1->Location = System::Drawing::Point(111, 336);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(141, 13);
			this->label1->TabIndex = 6;
			this->label1->Text = L"Jumlah Alat yang dinyalakan";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->ForeColor = System::Drawing::SystemColors::ButtonHighlight;
			this->label3->Location = System::Drawing::Point(270, 337);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(12, 13);
			this->label3->TabIndex = 7;
			this->label3->Text = L"/";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->ForeColor = System::Drawing::SystemColors::ButtonHighlight;
			this->label4->Location = System::Drawing::Point(111, 359);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(75, 13);
			this->label4->TabIndex = 8;
			this->label4->Text = L"Jumlah Harga ";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->ForeColor = System::Drawing::SystemColors::ButtonHighlight;
			this->label5->Location = System::Drawing::Point(270, 358);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(37, 13);
			this->label5->TabIndex = 9;
			this->label5->Text = L"50000";
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(0)),
				static_cast<System::Int32>(static_cast<System::Byte>(64)));
			this->ClientSize = System::Drawing::Size(923, 440);
			this->Controls->Add(this->label5);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->dataGridView1);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->button1);
			this->Name = L"MyForm";
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"LOL";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion



	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {

		if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{

			System::IO::StreamReader ^ sr = gcnew
				System::IO::StreamReader(openFileDialog1->FileName);
			System::String^ pathfilez = openFileDialog1->FileName;
			this->pathfile = (char*)(Marshal::StringToHGlobalAnsi(pathfilez)).ToPointer();
			input = this->pathfile;
			Solve();
			this->label2->Visible = true;
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(159, 19);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(35, 13);
			this->label2->TabIndex = 5;
			this->label2->Text = pathfilez;
			this->button2->Visible = true;
			this->button2->BackColor = System::Drawing::Color::Indigo;
			this->button2->Location = System::Drawing::Point(776, 12);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(84, 23);
			this->button2->TabIndex = 5;
			this->button2->Text = L"execute";
			this->button2->UseVisualStyleBackColor = false;
			// sr->Close();
		}

	}
	private: System::Void button2_Click(System::Object^ sender, System::EventArgs^  e) {

		this->ClientSize = System::Drawing::Size(923, 385);
		this->dataGridView1->RowCount = ndevice;
		for (int i = 0; i < ndevice; i++) {
			for (int j = 0; j < 1; j++) {
				this->dataGridView1->Rows[i]->Cells[j]->Value = gcnew System::String(devices[i].name.c_str());
				this->dataGridView1->Rows[i]->Cells[j + 1]->Value = devices[i].wajib;
			}
		}
		//for (int i = 0; i < ndevice; i++){
		// for (Interval& intv : devices[i].assignedRange) {
		// for (int j = intv.begin; j < intv.end; ++j){
		// this->dataGridView1->Rows[i]->Cells[j + 2]->Style->BackColor = System::Drawing::Color::Turquoise;
		// }
		// }
		//}

	}

	};
}

