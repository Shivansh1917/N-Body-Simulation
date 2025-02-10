import { useState, useEffect } from "react";
import dynamic from "next/dynamic";

// Dynamically import react-plotly.js with SSR disabled
const Plot = dynamic(() => import("react-plotly.js"), { ssr: false });

export default function Home() {
  const [messages, setMessages] = useState([]);

  useEffect(() => {
    const socket = new WebSocket("ws://localhost:8080");

    socket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      setMessages((prev) => {
        const index = prev.findIndex((msg) => msg.id === data.id);
        if (index !== -1) {
          const updated = [...prev];
          updated[index] = data;
          return updated;
        } else {
          return [...prev, data];
        }
      });
    };

    return () => socket.close();
  }, []);

  // Prepare data arrays for plotting
  const xData = messages.map((msg) => msg.x);
  const yData = messages.map((msg) => msg.y);

  return (
    <div>
      <h1>Live N-Body Data</h1>
      {messages.length > 0 ? (
        <Plot
          data={[
            {
              x: xData,
              y: yData,
              mode: "markers",
              type: "scatter",
              marker: { color: "blue", size: 8 },
              text: messages.map((msg) => `ID: ${msg.id}`),
            },
          ]}
          layout={{
            width: 800,
            height: 600,
            title: "Live Scatter Plot",
            xaxis: {
              title: "X Values",
              range: [-200, -200],       // Set fixed x-axis range (adjust as needed)
              fixedrange: true,     // Disables zooming/panning
            },
            yaxis: {
              title: "Y Values",
              range: [-200,-200],       // Set fixed y-axis range (adjust as needed)
              fixedrange: true,     // Disables zooming/panning
            },
          }}
        />
      ) : (
        <div>No data received yet.</div>
      )}
    </div>
  );
}
