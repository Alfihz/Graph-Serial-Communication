using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using rtChart;
using System.Windows.Forms.DataVisualization.Charting;
using System.IO;

namespace Distance_Sensor_using_Flight_of_time
{
    public partial class Form1 : Form
    {
        string recvData = "temporary";
        bool breakloop = false;
        kayChart chartData;
        bool buttonPress = false;
        int pointsCounter = 0;
        int firstpoint = 0;
        int secondpoint = 0;
        bool redLine = false;
        public string[] openData;

        ChartArea ca;
        Series s;
        public Form1()
        {
            InitializeComponent();
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            //chart1.Series.Add("Series1");
            //Connection COM & Baud Rate
            string[] ports = SerialPort.GetPortNames();
            string[] rates = new string[10] { "300", "600", "1200", "2400", "9600", "14400", "19200", "38400", "57600", "115200" };
            cboBaudRate.SelectedText = "9600";
            cboCOM.Items.AddRange(ports);
            cboBaudRate.Items.AddRange(rates);
            if (ports.Length >= 1)
                cboCOM.SelectedIndex = 0;

            //kayChart real time
            chartData = new kayChart(chart1, 60);

            btnStart.Enabled = false;
            //btnSave.Enabled = false;

            //my own chart
            ca = chart1.ChartAreas[0];
            s = chart1.Series[0];
        }

        private void BtnConnect_Click(object sender, EventArgs e)
        {
            try
            {
                if (btnConnect.Text == "Disconnect")
                {
                    if (btnStart.Text == "Stop")
                        MessageBox.Show("Please click \"Stop\" button first!");
                    else
                    {
                        serialPort1.Close();
                        btnStart.Enabled = false;
                        btnConnect.Text = "Connect";
                    }
                }
                else
                {
                    serialPort1.PortName = cboCOM.Text;
                    serialPort1.BaudRate = Convert.ToInt32(cboBaudRate.Text);
                    serialPort1.Parity = Parity.None;
                    serialPort1.StopBits = StopBits.One;
                    serialPort1.DataBits = 8;
                    serialPort1.Open();

                    btnStart.Enabled = true;
                    btnConnect.Text = "Disconnect";
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Message", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void serialDataReceive(object sender, SerialDataReceivedEventArgs e)
        {
            if (!breakloop)
            {
                SerialPort sData = sender as SerialPort;
                recvData = sData.ReadLine();

                string[] bufferData = recvData.Split(',');
                if (bufferData.Length == 2)
                    recvData = bufferData[1];
                //rtbData.Invoke((MethodInvoker)delegate {rtbData.AppendText(recvData); });

                //update chart
                if (bufferData[0] == "btnState")
                {
                    if (bufferData[1] == "true\r")
                        buttonPress = true;
                    else
                        buttonPress = false;
                }
                else if (bufferData[0] == "sData")
                {
                    double data;
                    bool result = Double.TryParse(recvData, out data);
                    if (result)
                    {
                        //chartData.TriggeredUpdate(data);
                        chart1.Invoke((MethodInvoker)delegate { s.Points.AddXY(pointsCounter, data); });
                        if (buttonPress == false)
                        {
                            //chart1.Invoke((MethodInvoker)delegate { chart1.Series["Series1"].Enabled = false; });
                            //chartData.serieName = "Length";
                            //chart1.Invoke((MethodInvoker)delegate { chart1.Series["Length"].Points.AddXY(pointsCounter, data); });
                            //chart1.Invoke((MethodInvoker)delegate { chart1.Series["Length"].Points[pointsCounter].Color = Color.Blue; });

                            chart1.Invoke((MethodInvoker)delegate { s.Points[pointsCounter].Color = Color.Blue; });
                        }
                        else
                        {
                            //chart1.Invoke((MethodInvoker)delegate { chart1.Series["Series1"].Enabled = true; });
                            //chartData.serieName = "Series1";
                            //chart1.Invoke((MethodInvoker)delegate { chart1.Series["Length"].Points.AddXY(pointsCounter, data); });
                            //chart1.Invoke((MethodInvoker)delegate { chart1.Series["Length"].Points[pointsCounter].Color = Color.Red; });

                            chart1.Invoke((MethodInvoker)delegate { s.Points[pointsCounter].Color = Color.Red; });
                            if (redLine == false)
                            {
                                firstpoint = pointsCounter;
                                redLine = true;
                            }
                            else
                            {
                                secondpoint = pointsCounter;
                            }
                        }

                        chart1.Invoke((MethodInvoker)delegate
                        {
                            //ix = s.Points.AddXY(pointsCounter, data);
                            if (pointsCounter > 20)
                            {
                                //ca.AxisX.Maximum = s.Points[pointsCounter].XValue;
                                //ca.AxisX.Minimum += s.Points[pointsCounter].XValue - s.Points[pointsCounter - 1].XValue;
                                ca.AxisX.Maximum = s.Points[pointsCounter].XValue + 10;
                                ca.AxisX.Minimum = s.Points[pointsCounter].XValue - 10;
                                ca.AxisY.Maximum = s.Points[pointsCounter].YValues[0] + 20;
                                ca.AxisY.Minimum = s.Points[pointsCounter].YValues[0] - 20;
                                //ca.AxisY.Maximum = 100;
                                //ca.AxisY.Minimum = 0;
                            }
                            ca.RecalculateAxesScale();
                            //ca.AxisX.ScaleView.Zoom(s.Points[ix - pointMax].XValue, s.Points[ix].XValue);
                        });

                        pointsCounter++;
                        //chartData.serieName = "Length";
                    }
                }
                rtbData.Invoke((MethodInvoker)delegate { rtbData.AppendText(recvData); });
            }
        }

        private void BtnStart_Click(object sender, EventArgs e)
        {
            try
            {
                if (btnStart.Text == "Start")
                {
                    btnSave.Enabled = true;
                    serialPort1.DataReceived += new SerialDataReceivedEventHandler(serialDataReceive);
                    btnStart.Text = "Stop";
                    breakloop = false;
                }
                else
                {
                    btnStart.Text = "Start";
                    breakloop = true;
                    //serialPort1.DataReceived += null;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Message", MessageBoxButtons.OK, MessageBoxIcon.Error);
                btnStart.Text = "Start";
            }
        }

        private void RtbData_TextChanged(object sender, EventArgs e)
        {
            rtbData.SelectionStart = rtbData.Text.Length;
            rtbData.ScrollToCaret();
        }

        private void Chart1_TextChanged(object sender, EventArgs e)
        {

        }

        private void BtnSave_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
            saveFileDialog1.Title = "Save data points";
            saveFileDialog1.DefaultExt = "txt";
            saveFileDialog1.ShowDialog();

            // If the file name is not an empty string open it for saving.
            if (saveFileDialog1.FileName != "")
            {
                using (StreamWriter sw = File.CreateText(saveFileDialog1.FileName))
                {
                    for (int i = firstpoint - 10; i < secondpoint + 10; i++)
                    {
                        if(i >= firstpoint && i <= secondpoint)
                            sw.WriteLine(s.Points[i].YValues[0].ToString() + ",red");
                        else
                            sw.WriteLine(s.Points[i].YValues[0].ToString() + ",blue");
                    }
                    sw.Close();
                }
                MessageBox.Show("Successfully saved the data!");
                saveFileDialog1.InitialDirectory = Path.GetDirectoryName(saveFileDialog1.FileName);
            }
        }

        private void BtnOpen_Click(object sender, EventArgs e)
        {
            List<string> list = new List<string>();
            string line;

            OpenFileDialog openFileDialog1 = new OpenFileDialog();
            openFileDialog1.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
            openFileDialog1.Title = "Open data points";
            openFileDialog1.DefaultExt = "txt";
            openFileDialog1.ShowDialog();
            
            if (openFileDialog1.FileName != "")
            {
                using (StreamReader sr = File.OpenText(openFileDialog1.FileName))
                {
                    while ((line = sr.ReadLine()) != null)
                    {
                        list.Add(line);
                    }
                    sr.Close();
                }
                openData = list.ToArray();
            }
        }
    }
}
