//! server

use axum::{Router, extract::State, response::Html, routing::get};
use std::net::SocketAddr;
use std::sync::Arc;

pub(crate) struct AppState {
    app_name: String,
    version: String,
    // TODO: some way to fetch simulation data here.
}

pub struct WebServer {
    addr: SocketAddr,
    state: Arc<AppState>,
}

impl WebServer {
    pub fn new(port: u16) -> Self {
        let addr = SocketAddr::from(([127, 0, 0, 1], port));
        let state = Arc::new(AppState {
            app_name: "Smoldyn".to_string(),
            version: env!("CARGO_PKG_VERSION").to_string(),
        });

        Self { addr, state }
    }

    pub async fn run(self) {
        // Build the router and attach the shared state
        let app = Router::new()
            .route("/", get(Self::root_handler))
            .route("/simdata", get(Self::simdata))
            .route("/version", get(Self::version))
            .with_state(self.state);

        // Bind the TCP listener
        let listener = tokio::net::TcpListener::bind(&self.addr).await.unwrap();

        println!("Server running on http://{}", self.addr);

        // Start the server
        axum::serve(listener, app).await.unwrap();
    }

    // generate the web-page here.
    async fn root_handler(State(_state): State<Arc<AppState>>) -> Html<String> {
        let index_page = include_str!("./index.html");

        Html(index_page.into())
    }

    // fetch simdata
    async fn simdata(State(_state): State<Arc<AppState>>) -> String {
        "simdata".to_string()
    }

    async fn version(State(state): State<Arc<AppState>>) -> String {
        state.version.clone()
    }
}
