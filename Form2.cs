using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading.Tasks;
using System.Windows.Forms.DataVisualization.Charting;

namespace Distance_Sensor_using_Flight_of_time
{
    public partial class Form2 : Form
    {
        public string[] openData { get; set; }

        public Form2()
        {
            InitializeComponent();
        }

        private void Form2_Load(object sender, EventArgs e)
        {
            Series s = chart1.Series[0];

            for(int i = 0; i < openData.Length; i++)
            {
                string recvData = openData[i];
                string[] bufferData = recvData.Split(',');
                if (bufferData.Length == 2)
                    recvData = bufferData[0];

                double data;
                bool result = Double.TryParse(recvData, out data);
                if (result)
                {
                    s.Points.AddXY(i, data);
                    if (bufferData[1] == "blue")
                    {
                        rtbData.SelectionColor = Color.Blue;
                        chart1.Invoke((MethodInvoker)delegate { s.Points[i].Color = Color.Blue; });
                    }
                    else
                    {
                        rtbData.SelectionColor = Color.Red;
                        chart1.Invoke((MethodInvoker)delegate { s.Points[i].Color = Color.Red; });
                    }

                }
                rtbData.AppendText(Environment.NewLine + recvData);
            }
        }

        private void RtbData_TextChanged(object sender, EventArgs e)
        {
            rtbData.SelectionStart = rtbData.Text.Length;
            rtbData.ScrollToCaret();
        }
    }
}
