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
    <div style={{ width: "100vw", height: "100vh", position: "relative" }}>
      {/* Optional heading overlay */}
      <h1 style={{ position: "absolute", zIndex: 2, margin: "10px", color: "#333" }}>
        Live N-Body Data
      </h1>
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
            autosize: true,
            title: "Live Scatter Plot",
            xaxis: {
              title: "X Values",
              range: [-500, 500],
              fixedrange: true,
            },
            yaxis: {
              title: "Y Values",
              range: [-500, 500],
              fixedrange: true,
            },
            margin: { t: 50, l: 50, r: 50, b: 50 }
          }}
          useResizeHandler={true}
          style={{ width: "100%", height: "100%" }}
        />
      ) : (
        <div style={{ textAlign: "center", paddingTop: "20px" }}>
          No data received yet.
        </div>
      )}
    </div>
  );
}
